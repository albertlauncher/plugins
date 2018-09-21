// Copyright (C) 2016-2017 Martin Buergmann

#include <QProcess>
#include "vmitem.h"
using namespace std;
using namespace Core;

/** ***************************************************************************/
QString VirtualBox::VMItem::iconPath_;
const int VirtualBox::VMItem::VM_START = 1;
const int VirtualBox::VMItem::VM_PAUSE = 2;
const int VirtualBox::VMItem::VM_RESUME = 3;
const int VirtualBox::VMItem::VM_STATE_CHANGING = -1;
const int VirtualBox::VMItem::VM_DIFFERENT = -2;

VirtualBox::VMItem::VMItem(const QString &name,
                           const QString &uuid,
                           int &mainAction,
                           const vector<shared_ptr<Action>> actions,
                           const QString &state)
    : name_(name),
      uuid_(uuid),
      idstring_(QString("extension.virtualbox.item:%1.%2").arg(uuid).arg(state)),
      actions_(actions),
      mainAction_(mainAction) { }

QString VirtualBox::VMItem::subtext() const {
    QString toreturn;
    switch (mainAction_) {
    case VM_START:
        toreturn = "Start %1";
        break;
    case VM_PAUSE:
        toreturn = "Pause %1";
        break;
    case VM_RESUME:
        toreturn = "Resume %1";
        break;
    case VM_STATE_CHANGING:
        toreturn = "The VM %1 is currently in action. Controls are disabled!";
        break;
    case VM_DIFFERENT:
        toreturn = "The VM %1 is currently in in unhandled state. Controls are disabled!";
        break;
    default:
        toreturn = "Start %1";
        break;
    }
    return toreturn.arg(name_);
}

/*
void VirtualBox::VMItem::activate(Action::ExecutionFlags *) {
    QString executionCommand;
    switch (mainAction_) {
    case VM_START:
        executionCommand = "vboxmanage startvm %1";
        break;
    case VM_PAUSE:
        executionCommand = "vboxmanage controlvm %1 pause";
        break;
    case VM_RESUME:
        executionCommand = "vboxmanage controlvm %1 resume";
        break;
    case VM_STATE_CHANGING:
        break;
    default:
        executionCommand = "vboxmanage startvm %1";
        break;
    }
    if (!executionCommand.isEmpty())
        QProcess::startDetached(executionCommand.arg(uuid_));
}
*/
