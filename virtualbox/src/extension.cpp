// Copyright (C) 2014-2018 Manuel Schneider
// Contributed to by 2016-2017 Martin Buergmann

#include <QDebug>
#include <QDomDocument>
#include <QDomElement>
#include <QFile>
#include <QListWidget>
#include <QPointer>
#include <QProcess>
#include <QRegularExpression>
#include <QMessageBox>
#include <QStandardPaths>
#include <QString>

#include <functional>
#include <nsIServiceManager.h>
#include <nsString.h>
#include <nsError.h>
#include "VirtualBox_XPCOM.h"

# include "nsEventQueueUtils.h"


#include "configwidget.h"
#include "extension.h"
#include "util/standarditem.h"
#include "util/standardactions.h"
#include "xdg/iconlookup.h"
using namespace std;
using namespace Core;


class VirtualBox::Private
{
public:

    QPointer<ConfigWidget> widget;

    nsCOMPtr<nsIComponentManager> manager;
    nsCOMPtr<IVirtualBox> virtualBox;
    std::map<QString, nsCOMPtr<IMachine>> virtualMachines;

    QString iconPath;

    void reloadVirtualMachines();
};


/** ***************************************************************************/
void VirtualBox::Private::reloadVirtualMachines() {

    virtualMachines.clear();

    // Get the virtual machines
    nsresult rc;
    IMachine **machines = nullptr;
    PRUint32 machineCnt = 0;
    rc = virtualBox->GetMachines(&machineCnt, &machines);
    if (NS_SUCCEEDED(rc)) {
        for (PRUint32 i = 0; i < machineCnt; ++ i) {
            IMachine *machine = machines[i];
            if (machine){

                PRBool isAccessible = PR_FALSE;
                machine->GetAccessible(&isAccessible);
                if (isAccessible) {

                    nsXPIDLString xpcomString;
                    machine->GetName(getter_Copies(xpcomString));
                    char *cstr = ToNewCString(xpcomString);
                    QString name(cstr);
                    free(cstr);

                    virtualMachines.emplace(name, machine);
                }
            }
        }
    }
}



/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
VirtualBox::Extension::Extension()
    : Core::Extension("org.albert.extension.virtualbox"),
      Core::QueryHandler(Core::Plugin::id()),
      d(new Private) {

    registerQueryHandler(this);

    nsresult rc;
    rc = NS_GetComponentManager(getter_AddRefs(d->manager));
    if (NS_FAILED(rc))
        throw("Error: could not get component manager.");

    rc = d->manager->CreateInstanceByContractID(NS_VIRTUALBOX_CONTRACTID,
                                                nullptr,
                                                NS_GET_IID(IVirtualBox),
                                                getter_AddRefs(d->virtualBox));
    if (NS_FAILED(rc))
        throw("Error, could not instantiate IVirtualBox object.");

    d->iconPath = XDG::IconLookup::iconPath("virtualbox");
    if ( d->iconPath.isNull() )
        d->iconPath = ":vbox";
}



/** ***************************************************************************/
VirtualBox::Extension::~Extension() {
    d->virtualBox = nullptr;
    NS_ShutdownXPCOM(nullptr);
}



/** ***************************************************************************/
QWidget *VirtualBox::Extension::widget(QWidget *parent) {
    if (d->widget.isNull()) {
        d->widget = new ConfigWidget(parent);

        d->widget->ui.listWidget->clear();
        for (auto &pair : d->virtualMachines)
            d->widget->ui.listWidget->addItem(pair.first);

        connect(d->widget->ui.pushButton, &QPushButton::clicked,
                this, [=](){d->reloadVirtualMachines();});
    }
    return d->widget;
}



/** ***************************************************************************/
void VirtualBox::Extension::setupSession() {
    d->reloadVirtualMachines();
}



/** ***************************************************************************/
void VirtualBox::Extension::handleQuery(Core::Query * query) const {

    if ( query->string().isEmpty() )
        return;

    for (auto &pair : d->virtualMachines) {
        if (pair.first.contains(query->string(), Qt::CaseInsensitive)){
            const QString &name = pair.first;
            nsCOMPtr<IMachine> &vm = pair.second;

            auto item = make_shared<StandardItem>(name);
            item->setIconPath(d->iconPath);
            item->setText(pair.first);
            item->setCompletion(pair.first);


            auto startVM = make_shared<FuncAction>("Start virtual machine", [this, vm]()
            {
                nsresult rc;
                nsCOMPtr<ISession> session;
                if (NS_SUCCEEDED(rc = d->manager->CreateInstanceByContractID(NS_SESSION_CONTRACTID, nullptr, NS_GET_IID(ISession), getter_AddRefs(session)))) {
                    nsCOMPtr<IProgress> progress;
                    if (NS_FAILED(rc = vm->LaunchVMProcess(session, QString("gui").utf16(), QString("").utf16(), getter_AddRefs(progress))))
                        qCritical() << "Error, could not start virtual machine:" << rc;
    //                                   << NS_ERROR_UNEXPECTED	<<"Virtual machine not registered."
    //                                   << NS_ERROR_INVALID_ARG	<<"Invalid session type."
    //                                   << VBOX_E_OBJECT_NOT_FOUND<<	"No machine matching machineId found."
    //                                   << VBOX_E_INVALID_OBJECT_STATE<<	"Session already open or being opened."
    //                                   << VBOX_E_IPRT_ERROR<<"	Launching process for machine failed."
    //                                   << VBOX_E_VM_ERROR<<"Failed to assign machine to session.";
                    session->UnlockMachine();
                } else
                    qCritical() << "Error, could not instantiate ISession object.";
            });

            auto saveVm = make_shared<FuncAction>("Save virtual machine", [this, vm]()
            {
                nsCOMPtr<ISession> session;
                nsresult rc;

                if (NS_SUCCEEDED(d->manager->CreateInstanceByContractID(NS_SESSION_CONTRACTID, nullptr, NS_GET_IID(ISession), getter_AddRefs(session)))){
                    if (NS_SUCCEEDED(vm->LockMachine(session, LockType_Shared))){
                        nsCOMPtr<IMachine> mutableVm;
                        if (NS_SUCCEEDED(session->GetMachine(getter_AddRefs(mutableVm)))){
                            nsCOMPtr<IProgress> progress;
                            if (NS_FAILED(rc = mutableVm->SaveState(getter_AddRefs(progress))))
                            qCritical() << "Error, failed saving state:" << rc;
                        } else
                            qCritical() << "Error, failed getting mutable IMachine of ISession.";
                    } else
                        qCritical() << "Error, could not acquire ISession lock.";
                    session->UnlockMachine();
                } else
                    qCritical() << "Error, could not instantiate ISession object.";
            });

            auto discardSavedVm = make_shared<FuncAction>("Discard saved state", [this, vm]()
            {
                nsCOMPtr<ISession> session;
                if (NS_SUCCEEDED(d->manager->CreateInstanceByContractID(NS_SESSION_CONTRACTID, nullptr, NS_GET_IID(ISession), getter_AddRefs(session)))){
                    if (NS_SUCCEEDED(vm->LockMachine(session, LockType_Shared))){
                        nsCOMPtr<IMachine> mutableVm;
                        if (NS_SUCCEEDED(session->GetMachine(getter_AddRefs(mutableVm)))){
                            if (NS_FAILED(mutableVm->DiscardSavedState(PR_TRUE)))
                                qCritical() << "Error, failed discarding saving state.";
                        } else
                            qCritical() << "Error, failed getting mutable IMachine of ISession.";
                    } else
                        qCritical() << "Error, could not acquire ISession lock.";
                    session->UnlockMachine();
                } else
                    qCritical() << "Error, could not instantiate ISession object.";
            });

            auto acpiPowerVm = make_shared<FuncAction>("Power off via ACPI event (Power button)", [this, vm]()
            {
                nsCOMPtr<ISession> session;
                if (NS_SUCCEEDED(d->manager->CreateInstanceByContractID(NS_SESSION_CONTRACTID, nullptr, NS_GET_IID(ISession), getter_AddRefs(session)))){
                    if (NS_SUCCEEDED(vm->LockMachine(session, LockType_Shared))){
                        nsCOMPtr<IConsole> console;
                        if (NS_SUCCEEDED(session->GetConsole(getter_AddRefs(console)))){
                            if (NS_FAILED(console->PowerButton()))
                                qCritical() << "Error, failed sending ACPI event (Power button).";
                        } else
                            qCritical() << "Error, failed getting IConsole of ISession.";
                    } else
                        qCritical() << "Error, could not acquire ISession lock.";
                    session->UnlockMachine();
                } else
                    qCritical() << "Error, could not instantiate ISession object.";
            });

            auto killVm = make_shared<FuncAction>("Turn off virtual machine", [this, vm]()
            {
                nsCOMPtr<ISession> session;
                if (NS_SUCCEEDED(d->manager->CreateInstanceByContractID(NS_SESSION_CONTRACTID, nullptr, NS_GET_IID(ISession), getter_AddRefs(session)))){
                    if (NS_SUCCEEDED(vm->LockMachine(session, LockType_Shared))){
                        nsCOMPtr<IConsole> console;
                        if (NS_SUCCEEDED(session->GetConsole(getter_AddRefs(console)))){
                            nsCOMPtr<IProgress> progress;
                            if (NS_FAILED(console->PowerDown(getter_AddRefs(progress))))
                                qCritical() << "Error, failed powering down machine.";
                        } else
                            qCritical() << "Error, failed getting IConsole of ISession.";
                    } else
                        qCritical() << "Error, could not acquire ISession lock.";
                    session->UnlockMachine();
                } else
                    qCritical() << "Error, could not instantiate ISession object.";
            });

            auto pauseVm = make_shared<FuncAction>("Pause virtual machine", [this, vm]()
            {
                nsCOMPtr<ISession> session;
                if (NS_SUCCEEDED(d->manager->CreateInstanceByContractID(NS_SESSION_CONTRACTID, nullptr, NS_GET_IID(ISession), getter_AddRefs(session)))){
                    if (NS_SUCCEEDED(vm->LockMachine(session, LockType_Shared))){
                        nsCOMPtr<IConsole> console;
                        if (NS_SUCCEEDED(session->GetConsole(getter_AddRefs(console)))){
                            if (NS_FAILED(console->Pause()))
                                qCritical() << "Error, failed pausing machine.";
                        } else
                            qCritical() << "Error, failed getting IConsole of ISession.";
                    } else
                        qCritical() << "Error, could not acquire ISession lock.";
                    session->UnlockMachine();
                } else
                    qCritical() << "Error, could not instantiate ISession object.";
            });

            auto resumeVm = make_shared<FuncAction>("Resume virtual machine", [this, vm]()
            {
                nsCOMPtr<ISession> session;
                if (NS_SUCCEEDED(d->manager->CreateInstanceByContractID(NS_SESSION_CONTRACTID, nullptr, NS_GET_IID(ISession), getter_AddRefs(session)))){
                    if (NS_SUCCEEDED(vm->LockMachine(session, LockType_Shared))){
                        nsCOMPtr<IConsole> console;
                        if (NS_SUCCEEDED(session->GetConsole(getter_AddRefs(console)))){
                            if (NS_FAILED(console->Resume()))
                                qCritical() << "Error, failed resuming machine.";
                        } else
                            qCritical() << "Error, failed getting IConsole of ISession.";
                    } else
                        qCritical() << "Error, could not acquire ISession lock.";
                    session->UnlockMachine();
                } else
                    qCritical() << "Error, could not instantiate ISession object.";
            });


            PRUint32 state;
            vm ->GetState(&state);
            switch (state) {
            case MachineState::PoweredOff:
                item->setSubtext("Machine is powered off");
                item->addAction(startVM);
                break;
            case MachineState::Saved:
                item->setSubtext("Machine is in saved state");
                item->addAction(startVM);
                item->addAction(discardSavedVm);
                break;
            case MachineState::Aborted:
                item->setSubtext("Machine is in aborted state");
                item->addAction(startVM);
                break;
            case MachineState::Running:
                item->setSubtext("Machine is running");
                item->addAction(saveVm);
                item->addAction(acpiPowerVm);
                item->addAction(killVm);
                item->addAction(pauseVm);
                break;
            case MachineState::Paused:
                item->setSubtext("Machine is in paused state");
                item->addAction(resumeVm);
                break;
            default:
                item->setSubtext("Machine is doing something. Please wait.");
                break;
            }

            query->addMatch(move(item), UINT_MAX);
        }
    }
}
