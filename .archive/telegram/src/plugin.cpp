// Copyright (c) 2023 Manuel Schneider

#include "QtCore/qdir.h"
#include "albert/util/standarditem.h"
#include "albert/albert.h"
#include "albert/logging.h"
#include "albert/query/action.h"
#include "plugin.h"
#include <td/telegram/Client.h>
#include <td/telegram/td_api.h>
#include <td/telegram/td_api.hpp>
#include <QInputDialog>
#include <QtConcurrent>
ALBERT_LOGGING_CATEGORY("tg")
using namespace albert;
using namespace std;
using namespace td::td_api;

// https://core.telegram.org/api/links#phone-number-links

// overloaded
namespace detail {
template <class... Fs>
struct overload;

template <class F>
struct overload<F> : public F {
    explicit overload(F f) : F(f) {
    }
};
template <class F, class... Fs>
struct overload<F, Fs...>
    : public overload<F>
    , public overload<Fs...> {
    overload(F f, Fs... fs) : overload<F>(f), overload<Fs...>(fs...) {
    }
    using overload<F>::operator();
    using overload<Fs...>::operator();
};
}  // namespace detail

template <class... F>
auto overloaded(F... f) {
    return detail::overload<F...>(f...);
}

Plugin::Plugin()
{
    td::ClientManager::execute(td_api::make_object<td_api::setLogVerbosityLevel>(1));
    client_manager_ = std::make_unique<td::ClientManager>();
    client_id_ = client_manager_->create_client_id();
    send_query(td_api::make_object<td_api::getOption>("version"), {});

    // Authorize in this thread
    while (!are_authorized_) {
        process_response(client_manager_->receive(10));
    }

    // Process further updates in backgronud
    QFuture<void> f = QtConcurrent::run([this] {
        while (are_authorized_)
        {
            if (auto response = client_manager_->receive(0); response.object)
            {
                std::lock_guard lock(mutex);
                process_response(std::move(response));
            }
            else
                QThread::sleep(1);
        }
    });
}

Plugin::~Plugin()
{
}

void Plugin::handleTriggerQuery(TriggerQuery *query) const
{
    std::lock_guard lock(mutex);
    for (const auto &[id, chat] : chats_)
    {
        CRIT << chat.title;
        if (chat.title.startsWith(query->string(), Qt::CaseInsensitive))
        {
            vector<Action> actions;
            if (auto it = users_.find(chat.user); it != users_.end())
            {
                const auto &[id, user] = *it;
                CRIT<< chat.title << user->phone_number_ << user->username_;
                actions.emplace_back(
                    "o",
                    "Open chat",
                    [n=user->phone_number_]{
                        albert::openUrl(QStringLiteral("tg://resolve?phone=") + QString::fromStdString(n));
                    }
                );
            } else
                CRIT << "No user found";

            query->add(albert::StandardItem::make(
                QString::number(id),
                chat.title,
                QString::number(id),
                {"qsp:SP_MessageBoxWarning"},
                actions
            ));
        }
    }
}


void Plugin::send_query(td_api::object_ptr<td_api::Function> f, std::function<void(Object)> handler)
{
    const auto query_id = ++current_query_id_;
    CRIT << QString("send_query #%1").arg(current_query_id_) << to_string(f);
    if (handler)
        response_handlers_.emplace(query_id, std::move(handler));
    client_manager_->send(client_id_, query_id, std::move(f));
}

void Plugin::process_response(td::ClientManager::Response response)
{
    if (!response.object)
        return;

    // CRIT << response.request_id << " " << to_string(response.object);

    if (response.request_id == 0)
        return process_update(std::move(response.object));

    if (auto it = response_handlers_.find(response.request_id); it != response_handlers_.end()) {
        it->second(std::move(response.object));
        response_handlers_.erase(it);
    }
}

void Plugin::process_update(td_api::object_ptr<td_api::Object> update)
{
    td_api::downcast_call(
        *update, overloaded(
            [this](td_api::updateAuthorizationState &u) {
                authorization_state_ = std::move(u.authorization_state_);
                on_authorization_state_update();
            },
            [this](td_api::updateNewChat &u) {
                                WARN << "updateNewChat"
                 << u.chat_->id_
                 << u.chat_->title_;

                auto &c = chats_[u.chat_->id_];
                c.id = u.chat_->id_;
                c.title = QString::fromStdString(u.chat_->title_);
                c.unread_count = u.chat_->unread_count_;
                td_api::downcast_call(
                    *u.chat_->type_, overloaded(
                        [&](td_api::chatTypePrivate &pc) {
                            c.user = pc.user_id_;
                        },
                        [&](auto &update) { c.user = 0; }));

                auto request = td_api::make_object<td_api::getChatHistory>(u.chat_->id_, 0, 0, 1, false);
                send_query(std::move(request), [this, t=u.chat_->title_](Object object)
                {
                    auto msgs = td::move_tl_object_as<td_api::messages>(object);
                    for (auto &msg : msgs->messages_) {
                        WARN << t << QString("tg://privatepost?channel=%1&post=%2").arg(msg->chat_id_).arg(msg->id_);
                    }
                });

            },
            [this](td_api::updateChatTitle &u) {
                WARN <<"updateChatTitle"<< u.chat_id_ << u.title_;
                chats_[u.chat_id_].title = QString::fromStdString(u.title_);
            },
            [this](td_api::updateUser &u) {
                                CRIT << "updateUser" << u.user_->phone_number_ << u.user_->username_;
                auto user_id = u.user_->id_;
                users_[user_id] = std::move(u.user_);
            },
            // [this](td_api::updateNewMessage &update_new_message) {
            //     auto chat_id = update_new_message.message_->chat_id_;
            //     std::string sender_name;
            //     td_api::downcast_call(*update_new_message.message_->sender_id_,
            //                           overloaded(
            //                               [this, &sender_name](td_api::messageSenderUser &user) {
            //                                   sender_name = get_user_name(user.user_id_);
            //                               },
            //                               [this, &sender_name](td_api::messageSenderChat &chat) {
            //                                   sender_name = get_chat_title(chat.chat_id_);
            //                               }));
            //     std::string text;
            //     if (update_new_message.message_->content_->get_id() == td_api::messageText::ID) {
            //         text = static_cast<td_api::messageText &>(*update_new_message.message_->content_).text_->text_;
            //     }
            //     INFO << "Receive message: [chat_id:" << chat_id << "] [from:" << sender_name << "] ["
            //               << text << "]";
            // },
            [](auto &update) {}));
}



auto Plugin::create_authentication_query_handler()
{
    CRIT << "create_authentication_query_handler";
    return [this, id = authentication_query_id_](Object object) {
        if (id == authentication_query_id_) {
            check_authentication_error(std::move(object));
        }
    };
}

void Plugin::check_authentication_error(Object object)
{
    CRIT << "check_authentication_error";
    if (object->get_id() == td_api::error::ID) {
        auto error = td::move_tl_object_as<td_api::error>(object);
        WARN << "Error: " << to_string(error);
        on_authorization_state_update();
    }
}

void Plugin::on_authorization_state_update()
{
    CRIT << "on_authorization_state_update";

    authentication_query_id_++;

    td_api::downcast_call(
        *authorization_state_,
        overloaded(
            [this](td_api::authorizationStateWaitTdlibParameters &)
            {
                DEBG << "authorizationStateWaitTdlibParameters";

                auto params = td_api::make_object<td_api::tdlibParameters>();
                params->database_directory_ = "tdlib";
                params->use_message_database_ = false;
                params->use_secret_chats_ = false;
                params->api_id_ = 1138533;
                params->api_hash_ = "a1a806a679788cb6103811d5b29cac90";
                params->system_language_code_ = "en"; // QLocale().language();
                params->device_model_ = "Desktop";
                params->system_version_ = "Albert Telegram Plugin";
                params->application_version_ = "1.0";

                auto request = td_api::make_object<td_api::setTdlibParameters>(std::move(params));
                send_query(std::move(request), create_authentication_query_handler());
            },
            [this](td_api::authorizationStateWaitEncryptionKey &) {
                DEBG << "authorizationStateWaitEncryptionKey";
                // Empty, nothing shoudl be stored anyway
                auto request = td_api::make_object<td_api::setDatabaseEncryptionKey>();
                send_query(std::move(request), create_authentication_query_handler());
            },
            [this](td_api::authorizationStateWaitPhoneNumber &) {
                DEBG << "authorizationStateWaitPhoneNumber";
                std::string phone_number("4917634565054");
                auto request = td_api::make_object<td_api::setAuthenticationPhoneNumber>(phone_number, nullptr);
                send_query(::move(request), create_authentication_query_handler());
            },
            [this](td_api::authorizationStateWaitCode &) {
                DEBG << "authorizationStateWaitCode";
                auto code = QInputDialog::getText(nullptr, "Telegram", "Enter code");
                send_query(td_api::make_object<td_api::checkAuthenticationCode>(code.toStdString()),
                           create_authentication_query_handler());
            },
            [this](td_api::authorizationStateReady &)
            {
                INFO << "Authorization is completed";
                are_authorized_ = true;
            },

            [this](td_api::authorizationStateLoggingOut &) {
                WARN << "authorizationStateLoggingOut (UNHANDLED)";
            },
            [this](td_api::authorizationStateClosing &) {
                WARN << "authorizationStateClosing (UNHANDLED)";
            },
            [this](td_api::authorizationStateClosed &) {
                WARN << "authorizationStateClosed (UNHANDLED)";
                are_authorized_ = false;
            },
            [this](td_api::authorizationStateWaitRegistration &) {
                WARN << "authorizationStateWaitRegistration (UNHANDLED)";
            },
            [this](td_api::authorizationStateWaitPassword &) {
                WARN << "authorizationStateWaitPassword (UNHANDLED)";
            },
            [this](td_api::authorizationStateWaitOtherDeviceConfirmation &state) {
                WARN << "authorizationStateWaitOtherDeviceConfirmation (UNHANDLED)";
            }
        )
    );
}


// std::string Plugin::get_user_name(std::int64_t user_id) const
// {
//     auto it = users_.find(user_id);
//     if (it == users_.end()) {
//         return "unknown user";
//     }
//     return it->second->first_name_ + " " + it->second->last_name_;
// }

// std::string Plugin::get_chat_title(std::int64_t chat_id) const
// {
//     auto it = chat_title_.find(chat_id);
//     if (it == chat_title_.end()) {
//         return "unknown chat";
//     }
//     return it->second;
// }

// #include <QThread>

// void Plugin::get_chats()
// {
//     send_query(td_api::make_object<td_api::getChats>(nullptr, 20),
//                [this](Object object)
//                {
//                    if (object->get_id() == td_api::error::ID)
//                        return;

//                    auto chats = td::move_tl_object_as<td_api::chats>(object);

//                    for (auto chat_id : chats->chat_ids_) {
//                        WARN << "[chat_id:" << chat_id  << "] [title:" << chat_title_[chat_id] << "]";
//                    }
//                });


//     while (1) {
//         auto response = client_manager_->receive(0);
//         if (response.object) {
//             process_response(std::move(response));
//         } else {
//             QThread::sleep(1);
//         }
//     }
// }











    // void loop() {
    //     while (true) {
    //         if (need_restart_) {
    //             restart();
    //         } else if (!are_authorized_) {
    //             process_response(client_manager_->receive(10));
    //         } else {
    //             std::cout << "Enter action [q] quit [u] check for updates and request results [c] show chats [m <chat_id> "
    //                          "<text>] send message [me] show self [l] logout: "
    //                      ;
    //             std::string line;
    //             std::getline(std::cin, line);
    //             std::istringstream ss(line);
    //             std::string action;
    //             if (!(ss >> action)) {
    //                 continue;
    //             }
    //             if (action == "q") {
    //                 return;
    //             }
    //             if (action == "u") {
    //                 std::cout << "Checking for updates...";
    //                 while (true) {
    //                     auto response = client_manager_->receive(0);
    //                     if (response.object) {
    //                         process_response(std::move(response));
    //                     } else {
    //                         break;
    //                     }
    //                 }
    //             } else if (action == "close") {
    //                 std::cout << "Closing...";
    //                 send_query(td_api::make_object<td_api::close>(), {});
    //             } else if (action == "me") {
    //                 send_query(td_api::make_object<td_api::getMe>(),
    //                            [this](Object object) { std::cout << to_string(object); });
    //             } else if (action == "l") {
    //                 std::cout << "Logging out...";
    //                 send_query(td_api::make_object<td_api::logOut>(), {});
    //             } else if (action == "m") {
    //                 std::int64_t chat_id;
    //                 ss >> chat_id;
    //                 ss.get();
    //                 std::string text;
    //                 std::getline(ss, text);

    //                 std::cout << "Sending message to chat " << chat_id << "...";
    //                 auto send_message = td_api::make_object<td_api::sendMessage>();
    //                 send_message->chat_id_ = chat_id;
    //                 auto message_content = td_api::make_object<td_api::inputMessageText>();
    //                 message_content->text_ = td_api::make_object<td_api::formattedText>();
    //                 message_content->text_->text_ = std::move(text);
    //                 send_message->input_message_content_ = std::move(message_content);

    //                 send_query(std::move(send_message), {});
    //             } else if (action == "c") {
    //                 std::cout << "Loading chat list...";
    //                 send_query(td_api::make_object<td_api::getChats>(nullptr, 20), [this](Object object) {
    //                     if (object->get_id() == td_api::error::ID) {
    //                         return;
    //                     }
    //                     auto chats = td::move_tl_object_as<td_api::chats>(object);
    //                     for (auto chat_id : chats->chat_ids_) {
    //                         std::cout << "[chat_id:" << chat_id << "] [title:" << chat_title_[chat_id] << "]";
    //                     }
    //                 });
    //             }
    //         }
    //     }
    // }
