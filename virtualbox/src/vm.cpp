// Copyright (C) 2016-2017 Martin Buergmann

#include "vm.h"

#include <QDomDocument>
#include <QDomElement>
#include <QFile>
#include <QObject>
#include <QProcess>
#include <QRegularExpression>
#include <nsString.h>
#include <nsIServiceManager.h>
#include <util/standardactions.h>
using namespace Core;
using namespace std;


/** ***************************************************************************/
VirtualBox::VM::VM(IMachine *machine) : machine_(machine) {
    PRBool isAccessible = PR_FALSE;
    machine->GetAccessible(&isAccessible);

    if (isAccessible) {
        nsXPIDLString machineName;
        machine->GetName(getter_Copies(machineName));
        char *machineNameAscii = ToNewCString(machineName);
        name_ = machineNameAscii;
        free(machineNameAscii);
    } else {
        name_ = "<inaccessible>";
    }

    nsXPIDLString iid;
    machine->GetId(getter_Copies(iid));
    const char *uuidString = ToNewCString(iid);
    uuid_ = uuidString;
    free((void*)uuidString);
}



/** ***************************************************************************/
VirtualBox::VM::~VM() {
    /* don't forget to release the objects in the array... */
    machine_->Release();
}



/*

  A state diagram of the VBox VMStates

            +---------[powerDown()] <- Stuck <--[failure]-+
            V                                             |
    +-> PoweredOff --+-->[powerUp()]--> Starting --+      | +-----[resume()]-----+
    |                |                             |      | V                    |
    |   Aborted -----+                             +--> Running --[pause()]--> Paused
    |                                              |      ^ |                   ^ |
    |   Saved -----------[powerUp()]--> Restoring -+      | |                   | |
    |     ^                                               | |                   | |
    |     |     +-----------------------------------------+-|-------------------+ +
    |     |     |                                           |                     |
    |     |     +- OnlineSnapshotting <--[takeSnapshot()]<--+---------------------+
    |     |                                                 |                     |
    |     +-------- Saving <--------[saveState()]<----------+---------------------+
    |                                                       |                     |
    +-------------- Stopping -------[powerDown()]<----------+---------------------+

 */
/** ***************************************************************************/
VirtualBox::VMItem *VirtualBox::VM::produceItem() const {

    QStringList pauseCommandline  = {"VBoxManage", "controlvm", uuid_, "pause"};
    QStringList startCommandline  = {"VBoxManage", "startvm",   uuid_};
    QStringList saveCommandline   = {"VBoxManage", "controlvm", uuid_, "savestate"};
    QStringList stopCommandline   = {"VBoxManage", "controlvm", uuid_, "poweroff"};
    QStringList resetCommandline  = {"VBoxManage", "controlvm", uuid_, "reset"};
    QStringList resumeCommandline = {"VBoxManage", "controlvm", uuid_, "resume"};
    vector<shared_ptr<Action>> actions;
    int mainAction = 0;

    PRUint32 state;
    machine_->GetState(&state);
    switch (state) {
    case MachineState::Starting:
    case MachineState::Restoring:
    case MachineState::Saving:
    case MachineState::Stopping:
        mainAction = VMItem::VM_STATE_CHANGING;
        actions.push_back(make_shared<FuncAction>("Controls are disabled", [](){}));
        break;
    case MachineState::PoweredOff:
    case MachineState::Aborted:
        mainAction = VMItem::VM_START;
        actions.push_back(make_shared<ProcAction>("Start", startCommandline));
        break;
    case MachineState::Saved:
        mainAction = VMItem::VM_START;
        actions.push_back(make_shared<ProcAction>("Start", startCommandline));
        break;
    case MachineState::Running:
        mainAction = VMItem::VM_PAUSE;
        actions.push_back(make_shared<ProcAction>("Pause", pauseCommandline));
        actions.push_back(make_shared<ProcAction>("Save State", saveCommandline));
        actions.push_back(make_shared<ProcAction>("Stop", stopCommandline));
        break;
    case MachineState::Paused:
        mainAction = VMItem::VM_RESUME;
        actions.push_back(make_shared<ProcAction>("Resume", resumeCommandline));
        actions.push_back(make_shared<ProcAction>("Save State", saveCommandline));
        actions.push_back(make_shared<ProcAction>("Reset", resetCommandline));
        break;
    default:
        mainAction = VMItem::VM_DIFFERENT;
        actions.push_back(make_shared<FuncAction>("Controls are disabled", [](){}));
    }

    return new VMItem(name_, uuid_, mainAction, actions, state_);
}



/** ***************************************************************************/
bool VirtualBox::VM::startsWith(QString other) const {
    return name_.startsWith(other, Qt::CaseInsensitive);
}



/** ***************************************************************************
void VirtualBox::VM::probeState() const {
    QProcess *process = new QProcess;
    process->setReadChannel(QProcess::StandardOutput);
    process->start("VBoxManage",  {"showvminfo", uuid_, "--machinereadable"});
    QObject::connect(process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
        [this, process](int exitCode, QProcess::ExitStatus exitStatus){
        if (exitStatus == QProcess::NormalExit && exitCode == 0){
            while (process->canReadLine()) {
               QString line = QString::fromLocal8Bit(process->readLine());
               if (line.startsWith("VMState=")) {
                   QRegularExpression regex("VMState=\"(.*)\"");
                   QRegularExpressionMatch match = regex.match(line);
                   state_ = match.captured(1).toLower();
                   break;
               }
            }
        }
        process->deleteLater();
    });
    QObject::connect(process, static_cast<void(QProcess::*)(QProcess::ProcessError)>(&QProcess::error), [process](QProcess::ProcessError){
        process->deleteLater();
    });
}*/

