// Copyright (c) 2024 Manuel Schneider
#pragma once
#include "albert/util/extensionplugin.h"
#include "albert/query/triggerqueryhandler.h"
#include <memory>
#include <mutex>
#include <td/telegram/Client.h>
#include <td/telegram/td_api.h>
#include <td/telegram/td_api.hpp>
namespace td { class ClientManager; }
namespace td_api = td::td_api;

struct Chat
{
public:
    std::int64_t id;
    QString title;
    int32_t unread_count;
    int64_t user;
};

class Plugin : public albert::ExtensionPlugin, public albert::TriggerQueryHandler
{
    Q_OBJECT ALBERT_PLUGIN

public:
    Plugin();
    ~Plugin() override;

    void handleTriggerQuery(TriggerQuery*) const override;

private:
    using Object = td_api::object_ptr<td_api::Object>;
    void send_query(td_api::object_ptr<td_api::Function> f, std::function<void (Object)> handler);
    void process_response(td::ClientManager::Response response);
    void process_update(td_api::object_ptr<td_api::Object> update);
    void on_authorization_state_update();
    void check_authentication_error(Object object);
    auto create_authentication_query_handler();



    mutable std::mutex mutex;
    // std::vector<Chat> chats_;

    std::unique_ptr<td::ClientManager> client_manager_;
    std::int32_t client_id_{0};
    td_api::object_ptr<td_api::AuthorizationState> authorization_state_;

    std::uint64_t current_query_id_{0};
    std::uint64_t authentication_query_id_{0};

    bool are_authorized_{false};

    std::map<std::uint64_t, std::function<void(Object)>> response_handlers_;
    std::map<std::int64_t, td_api::object_ptr<td_api::user>> users_;
    std::map<std::int64_t, Chat> chats_;
};
