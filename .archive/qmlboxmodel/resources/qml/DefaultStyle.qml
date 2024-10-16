import Qt5Compat.GraphicalEffects
import QtQml.StateMachine as DSM  // State name clashes.
import QtQuick
import QtQuick.Window
import "albert.js" as Util

Item
{
    DebugRect{ name: "root" }

    id: root
    objectName: "root"

    property bool debug: false

    property alias inputText: input.text
    signal inputTextEdited(string text)



    QtObject
    {
        id: style
        objectName: "style"

        // General
        property int  animation_duration: 100
        // property font font_family: Qt.application.font  // custom fonts led to weird padding on some systems

        // Shadow
        property color  shadow_color: "#80000000"
        property int    shadow_size: 80
        property int    shadow_offset: 10

        // Frame
        property color  window_background_color: palette.window
        property color  window_border_color: palette.base
        property int    window_border_radius: 17
        property real   window_border_width: 0.5
        property int    window_content_padding: 6
        property int    window_content_spacing: 6
        property int    window_width: 640

        // Input
        property color  input_background_color: palette.base
        property color  input_border_color: 'transparent'
        property color  input_cursor_color: palette.text
        property color  input_hint_color: palette.placeholderText
        property color  input_text_color: palette.text
        property color  input_text_selection_background_color: palette.highlight
        property color  input_text_selection_color: palette.highlightedText
        property color  input_text_trigger_color: palette.highlight
        property int    input_border_width: 0
        property int    input_border_radius: 10
        property int    input_padding: 4
        property int    input_text_font_size: 23

        // Settings button
        property color settings_button_color: palette.window
        property color settings_button_color_highlight: palette.highlight
        property int   settings_button_margin: 14
        property int   settings_button_size: 17

        // Results list
        property color result_list_highlight_background_color: palette.highlight
        property color result_list_highlight_border_color: palette.highlight
        property int   result_list_highlight_border_width: 0
        property int   result_list_highlight_radius: 10
        property int   result_list_max_visible: 5
        property int   result_list_padding: 0
        property int   result_list_spacing: 0

        // Result item
        property color result_item_subtext_color: palette.placeholderText
        property color result_item_subtext_color_current: palette.highlightedText
        property color result_item_text_color: palette.text
        property color result_item_text_color_current: palette.highlightedText
        property int   result_item_icon_size: 30
        property int   result_item_padding: 6
        property int   result_item_spacing_horizontal: 8
        property int   result_item_spacing_vertical: 0
        property int   result_item_text_font_size: 16
        property int   result_item_subtext_font_size: 10

        // Actions list
        property color action_list_highlight_background_color: palette.highlight
        property color action_list_highlight_border_color: palette.highlight
        property int   action_list_highlight_border_radius: 10
        property int   action_list_highlight_border_width: 0
        property int   action_list_padding: 0
        property int   action_list_spacing: 0

        // Action item
        property color action_item_text_color: palette.text
        property color action_item_text_color_current: palette.highlightedText
        property int   action_item_padding: 4
        property int   action_item_text_font_size: 14
    }

    width: frame.width + style.shadow_size * 2
    height: frame.height + style.shadow_size * 2 + style.shadow_offset


    // Item {
    //     id: shadowItem
    //     layer.enabled: style.shadow_size > 0
    //     // layer.live: false
    //     layer.effect: Loader {
    //         active: style.shadow_size > 0
    //         // anchors.fill: frame
    //         sourceComponent: DropShadow {
    //             id: shadow
    //             source: frame
    //             transparentBorder: true
    //             verticalOffset: style.shadow_offset
    //             radius: style.shadow_size
    //             color: style.shadow_color
    //             // spread: 0.1
    //             // cached: true

    //             // samples: Not available in below 6.4. Branch and set dynamically
    //             Component.onCompleted: {
    //                 if (QT_VERSION_MAJOR === 6 && QT_VERSION_MINOR >= 4)
    //                     shadow.samples = style.shadow_size * 2
    //             }
    //         }
    //     }
    // }

    Loader {
        active: style.shadow_size > 0
        anchors.fill: frame
        sourceComponent: DropShadow {
            id: shadow
            source: frame
            transparentBorder: true
            verticalOffset: style.shadow_offset
            radius: style.shadow_size
            color: style.shadow_color
            // spread: 0.1
            // cached: true

            // samples: Not available in below 6.4. Branch and set dynamically
            Component.onCompleted: {
                if (QT_VERSION_MAJOR === 6 && QT_VERSION_MINOR >= 4)
                    shadow.samples = style.shadow_size * 2
            }
        }
    }

    Rectangle
    {
        DebugRect{ name: "frame" }
        id: frame

        x: style.shadow_size
        y: style.shadow_size
        width: style.window_width
        height: contentColumn.height

        color: style.window_background_color
        border.color: style.window_border_color
        border.width: style.window_border_width
        radius: style.window_border_radius

        Column
        {
            id: contentColumn

            width: frame.width
            padding: style.window_content_padding + style.window_border_width
            spacing: style.window_content_spacing

            Rectangle
            {
                id: inputFrame
                DebugRect{ name: "inputFrame" }

                height: input.height + style.input_padding * 2
                width: contentColumn.width - (style.window_content_padding + style.window_border_width) * 2

                color: style.input_background_color
                border.color: style.input_border_color
                border.width: style.input_border_width
                radius: style.input_border_radius

                MouseArea {
                    id: inputFrameMouseArea
                    DebugRect{ name: "inputFrameMouseArea" }

                    anchors.fill: parent

                    hoverEnabled: true

                    HistoryTextInput
                    {
                        id: input
                        DebugRect{ name: "HistoryTextInput" }

                        anchors.left: parent.left
                        anchors.leftMargin: style.input_padding
                        anchors.right: parent.right
                        anchors.rightMargin: style.input_padding
                        anchors.verticalCenter: parent.verticalCenter

                        font.pixelSize: style.input_text_font_size
                        color: style.input_text_color
                        cursorColor: style.input_cursor_color
                        selectedTextColor: style.input_text_selection_color
                        selectionColor: style.input_text_selection_background_color
                        triggerColor: style.input_text_trigger_color

                        Keys.forwardTo: [resultsList]
                    }

                    Text
                    {
                        DebugRect{ name: "hint" }
                        id: inputHint

                        x: input.contentWidth + style.input_padding
                        anchors.right: parent.right
                        anchors.rightMargin: style.input_padding
                        anchors.verticalCenter: parent.verticalCenter

                        color: style.input_hint_color
                        font: input.font
                        elide: Text.ElideLeft
                    }

                    DebugRect{ target: settingsButton; name: "sb" }
                    SettingsButton
                    {
                        id: settingsButton

                        width: input.height; height: input.height
                        anchors.right: parent.right
                        anchors.rightMargin: style.input_padding
                        anchors.verticalCenter: parent.verticalCenter

                        size: style.settings_button_size
                        visible: opacity > 0
                        Behavior on opacity { NumberAnimation { } }
                    }
                }
            }

            DesktopListView
            {
                DebugRect{ name: "resultsList" }
                id: resultsList
                width: contentColumn.width - contentColumn.padding*2 - style.result_list_padding*2
                anchors.horizontalCenter: parent.horizontalCenter

                maxItems: style.result_list_max_visible
                spacing: style.result_list_spacing
                delegate: ResultItemDelegate
                {
                    DebugRect{ name: "resultItem" }
                    iconSize: style.result_item_icon_size
                    text_font_size: style.result_item_text_font_size
                    subtext_font_size: style.result_item_subtext_font_size
                    text_color: ListView.isCurrentItem ? style.result_item_text_color_current : style.result_item_text_color
                    subtext_color: ListView.isCurrentItem ? style.result_item_subtext_color_current : style.result_item_subtext_color
                    padding: style.result_item_padding
                    spacing_horizontal: style.result_item_spacing_horizontal
                    spacing_vertical: style.result_item_spacing_vertical
                    onClicked: (result_index) => { resultsList.itemActivated(result_index) }
                }
                highlight: Rectangle
                {
                    DebugRect{ name: "highlight" }
                    color: style.result_list_highlight_background_color
                    border.color: style.result_list_highlight_border_color
                    border.width: style.result_list_highlight_radius
                    radius: style.result_list_highlight_radius
                }
                onItemActivated: (result_index) => { activate(result_index) }
                Keys.forwardTo: [root]

                // onCurrentIndexChanged: if (currentIndex >= 0) setInputActionHint(currentItem.getInputAction())
                //Keys.onPressed: (event)=>{ Util.printKeyPress("resultsList", event) }
                // Component.onCompleted: {albert.debug("reuseItems" + reuseItems)}
            }

            DesktopListView { DebugRect{ name: "actionsList" }
                id: actionsList
                width: contentColumn.width - contentColumn.padding*2 - style.action_list_padding*2
                anchors.horizontalCenter: parent.horizontalCenter
                onCountChanged: height = (count === 0) ? 0 : count * (contentItem.children[0].height + spacing) - spacing
                visible: count !== 0
                spacing: style.action_list_spacing

                delegate: ActionItemDelegate { DebugRect{ name: "actionItem" }
                    color: style.action_item_text_color
                    color_current: style.action_item_text_color_current
                    font_size: style.action_item_text_font_size
                    padding: style.action_item_padding
                    onClicked: (action_index) => { actionsList.itemActivated(action_index) }
                }
                highlight: Rectangle { DebugRect{ name: "highlight" }
                    color: style.action_list_highlight_background_color
                    border.color: style.action_list_highlight_border_color
                    border.width: style.action_list_highlight_border_radius
                    radius: style.action_list_highlight_border_radius
                }
                onItemActivated: (action_index) => { activate(resultsList.currentIndex, action_index) }
                //Keys.onPressed: (event)=>{ Util.printKeyPress("actionsList", event) }
                Keys.forwardTo: [root]
            }
        }
    }

    DSM.StateMachine {
        id: statemachine
        running: true
        initialState: sRoot

        signal enterActionsMode
        signal exitActionsMode
        signal toggleActionsMode
        signal enterFallbackMode
        signal exitFallbackMode

        component StateDebug: DSM.State {
            property string name
            onEntered: albert.debug("ENTER " + name)
            onExited: albert.debug("EXIT " + name)
        }

        DSM.State {
            id: sRoot
            childMode: DSM.QState.ParallelStates

            DSM.State {
                initialState: sSettingsButtonHidden

                StateDebug { name: "sSettingsButtonHidden"
                    id: sSettingsButtonHidden;

                    onEntered: settingsButton.opacity = 0

                    DSM.SignalTransition {
                        targetState: sSettingsButtonVisible
                        signal: inputFrameMouseArea.entered
                    }

                    DSM.SignalTransition {
                        targetState: sSettingsButtonHighlight
                        signal: albert.queryChanged
                        guard: albert.currentQuery() !== null
                    }

                    DSM.SignalTransition {
                        targetState: sSettingsButtonHighlight
                        signal: settingsButton.entered
                    }
                }

                StateDebug { name: "sSettingsButtonVisible"
                    id: sSettingsButtonVisible;

                    onEntered: {
                        settingsButton.opacity = 1
                        settingsButton.color = style.settings_button_color
                    }

                    DSM.SignalTransition {
                        targetState: sSettingsButtonHidden
                        signal: inputFrameMouseArea.exited
                    }

                    DSM.SignalTransition {
                        targetState: sSettingsButtonHighlight
                        signal: albert.queryChanged
                        guard: albert.currentQuery() !== null
                    }

                    DSM.SignalTransition {
                        targetState: sSettingsButtonHighlight
                        signal: settingsButton.entered
                    }
                }

                StateDebug { name: "sSettingsButtonHighlight"
                    id: sSettingsButtonHighlight;

                    onEntered: {
                        settingsButton.opacity = 1
                        settingsButton.color = style.settings_button_color_highlight
                    }

                    DSM.SignalTransition {
                        targetState: sSettingsButtonVisible
                        signal: settingsButton.exited
                        guard: inputFrameMouseArea.containsMouse
                               && (albert.currentQuery() === null
                                   || albert.currentQuery().isFinished())
                    }

                    DSM.SignalTransition {
                        targetState: sSettingsButtonVisible
                        signal: albert.queryChanged
                        guard: inputFrameMouseArea.containsMouse
                               && albert.currentQuery() === null
                               && !settingsButton.containsMouse
                    }

                    DSM.SignalTransition {
                        targetState: sSettingsButtonVisible
                        signal: albert.queryFinished
                        guard: inputFrameMouseArea.containsMouse
                               && !settingsButton.containsMouse
                    }


                    DSM.SignalTransition {
                        targetState: sSettingsButtonHidden
                        signal: settingsButton.exited
                        guard: !inputFrameMouseArea.containsMouse
                               && (albert.currentQuery() === null
                                   || albert.currentQuery().isFinished())
                    }

                    DSM.SignalTransition {
                        targetState: sSettingsButtonHidden
                        signal: albert.queryChanged
                        guard: !inputFrameMouseArea.containsMouse
                               && albert.currentQuery() === null
                               && !settingsButton.containsMouse
                    }

                    DSM.SignalTransition {
                        targetState: sSettingsButtonHidden
                        signal: albert.queryFinished
                        guard: !inputFrameMouseArea.containsMouse
                               && !settingsButton.containsMouse
                    }
                }
            }

            DSM.State {
                initialState: sQueryUnset

                StateDebug { name: "sQueryUnset"
                    id: sQueryUnset;

                    onEntered: {
                        resultsList.visible = false
                        resultsList.model = null
                    }

                    DSM.SignalTransition {
                        targetState: sQuerySet
                        signal: albert.queryChanged
                        guard: albert.currentQuery() !== null
                    }
                }

                StateDebug { name: "sQuerySet"
                    id: sQuerySet;
                    initialState: sResultsHidden

                    DSM.SignalTransition {
                        targetState: sQueryUnset
                        signal: albert.queryChanged
                        guard: albert.currentQuery() === null
                    }

                    StateDebug {name: "sResultsHidden"
                        id: sResultsHidden

                        onEntered: resultsList.visible = false

                        DSM.SignalTransition {
                            targetState: sResultsMatches
                            signal: albert.queryMatchesAdded
                        }

                        DSM.SignalTransition {
                            targetState: sResultsFallbacks
                            signal: statemachine.enterFallbackMode
                            guard: albert.currentQuery().fallbacks().rowCount() > 0
                        }

                        DSM.SignalTransition {
                            targetState: sResultsFallbacks
                            signal: albert.queryFinished
                            guard: !albert.currentQuery().isTriggered()
                                   && albert.currentQuery().fallbacks().rowCount() > 0
                        }
                    }

                    StateDebug {name: "sResultsDisabled"
                        id: sResultsDisabled

                        onEntered: {
                            resultsList.enabled = false
                            actionsList.enabled = false
                        }
                        onExited: {
                            resultsList.enabled = true
                            actionsList.enabled = true
                        }

                        DSM.TimeoutTransition {
                            targetState: sResultsHidden
                            timeout: 250
                        }

                        DSM.SignalTransition {
                            targetState: sResultsMatches
                            signal: albert.queryMatchesAdded
                        }

                        DSM.SignalTransition {
                            targetState: sResultsHidden
                            signal: albert.queryFinished
                            guard: albert.currentQuery().isTriggered()
                                   || albert.currentQuery().fallbacks().rowCount() === 0
                        }

                        DSM.SignalTransition {
                            targetState: sResultsFallbacks
                            signal: albert.queryFinished
                            guard: !albert.currentQuery().isTriggered()
                                   && albert.currentQuery().fallbacks().rowCount() > 0
                        }
                    }

                    DSM.State {
                        id: sResultsMatches
                        initialState: sResultsMatchItems

                        onEntered:{
                            resultsList.model = albert.currentQuery().matches()
                            resultsList.visible = true
                        }

                        DSM.SignalTransition {
                            targetState: sResultsDisabled
                            signal: albert.queryChanged
                            guard: albert.currentQuery() !== null
                        }

                        DSM.SignalTransition {
                            targetState: sResultsFallbacks
                            signal: statemachine.enterFallbackMode
                            guard: albert.currentQuery().fallbacks().rowCount() > 0
                        }

                        StateDebug {name: "sResultsMatchItems"
                            id: sResultsMatchItems

                            DSM.SignalTransition { targetState: sResultsMatchActions; signal: statemachine.enterActionsMode }
                            DSM.SignalTransition { targetState: sResultsMatchActions; signal: statemachine.toggleActionsMode }
                        }

                        StateDebug {name: "sResultsMatchActions"
                            id: sResultsMatchActions
                            onEntered: {
                                actionsList.model = albert.createStringListModel(resultsList.currentItem.getActionsList())
                                input.Keys.forwardTo = [actionsList]
                            }
                            onExited: {
                                actionsList.model = null
                                input.Keys.forwardTo = [resultsList]
                            }

                            DSM.SignalTransition { targetState: sResultsMatchItems; signal: statemachine.exitActionsMode }
                            DSM.SignalTransition { targetState: sResultsMatchItems; signal: statemachine.toggleActionsMode }
                        }
                    }

                    DSM.State {
                        id: sResultsFallbacks
                        initialState: sResultFallbackItems

                        onEntered:{
                            resultsList.model = albert.currentQuery().fallbacks()
                            resultsList.visible = true
                        }

                        DSM.SignalTransition {
                            targetState: sResultsDisabled
                            signal: albert.queryChanged
                            guard: albert.currentQuery() !== null
                        }

                        DSM.SignalTransition {
                            targetState: sResultsHidden
                            signal: statemachine.exitFallbackMode
                            guard: albert.currentQuery().matches().rowCount() === 0
                                   || !albert.currentQuery().isFinished()
                        }

                        DSM.SignalTransition {
                            targetState: sResultsMatches
                            signal: statemachine.exitFallbackMode
                            guard: albert.currentQuery().matches().rowCount() > 0
                        }

                        StateDebug {name: "sResultFallbackItems"
                            id: sResultFallbackItems

                            // DSM.SignalTransition { targetState: sResultsFallbackActions; signal: statemachine.enterActionsMode }
                            // DSM.SignalTransition { targetState: sResultsFallbackActions; signal: statemachine.toggleActionsMode }
                        }

                        StateDebug {name: "sResultsFallbackActions"
                            id: sResultsFallbackActions

                            // DSM.SignalTransition { targetState: sResultFallbackItems; signal: statemachine.exitActionsMode }
                            // DSM.SignalTransition { targetState: sResultFallbackItems; signal: statemachine.toggleActionsMode }
                        }
                    }
                }
            }
        }
    }

    Connections {
        target: albert
        function onQueryChanged() {
            if (albert.currentQuery() !== null)
            {
                let q = albert.currentQuery()
                albert.debug(`onQueryChanged: T ${q.trigger()} S ${q.string()} SYN ${q.synopsis()}`)
                input.triggerLength = q.trigger().length
                inputHint.text = q.synopsis()
            }
            else
            {
                albert.debug("onQueryChanged: null")
                input.triggerLength = 0
                inputHint.text = ''
            }

        }
    }

    Connections { target: albert; function onQueryMatchesAdded() { albert.debug("onQueryMatchesAdded") } }
    Connections { target: albert; function onQueryFinished() { albert.debug("onQueryFinished") } }
    Connections { target: input; function onTextEdited() { inputTextEdited(input.text) }}

    Keys.onPressed: (event)=> {
        //Util.printKeyPress("root", event)

        if (Util.testKey(event, Qt.Key_Meta))
            statemachine.enterFallbackMode()

        else if (Util.testKey(event, Qt.Key_Alt))
            statemachine.enterActionsMode()

        else if (Util.testAnyKeyCombination(event, Qt.ControlModifier, [Qt.Key_Enter, Qt.Key_Return]))
            statemachine.toggleActionsMode()

        else if (Util.testKeyCombination(event, Qt.ShiftModifier, Qt.Key_Up)
                 || sResultsMatchItems.active && (resultsList.currentIndex < 1 && !event.isAutoRepeat)  // is first item
                    && Util.testKey(event, Qt.Key_Up))
            input.historyNext()

        else if (sResultsMatchItems.active && Util.testKeyCombination(event, Qt.ShiftModifier, Qt.Key_Down))
            input.historyPrev()

        else if (Util.testKeyCombination(event, Qt.ControlModifier, Qt.Key_Comma))
            albert.showSettings()

        else if (resultsList.currentIndex >= 0 && Util.testKeyCombination(event, Qt.NoModifier, Qt.Key_Tab))
            applyInputAction(resultsList.currentItem.getInputAction())
    }

    Keys.onReleased: (event)=> {
        if (Util.testKey(event, Qt.Key_Meta))
            statemachine.exitFallbackMode()

        else if (Util.testKey(event, Qt.Key_Alt))
            statemachine.exitActionsMode()
    }

    function applyInputAction(inputActionText) {
        if (inputActionText.length !== 0){
            input.text = /*albert.currentQuery().trigger() +*/ inputActionText
            input.onTextEdited()
        }
    }

    function setInputActionHint(inputActionText) {
        let query = albert.currentQuery()
        if (query.string().length !== 0){

            // Todo this feature breaaks a lot of handlers, cool thoguh, do it
            // let replacement = albert.currentQuery().trigger() + inputActionText
            let replacement = inputActionText

            if (inputActionText.length === 0)
                hint.text = ""
            else if (replacement.toLowerCase().startsWith(input.text.toLowerCase()))
                hint.text = replacement.slice(input.length, inputActionText.length)
            else
                hint.text = "  " + replacement
        }
    }

    // Window.onVisibilityChanged: {
    //     Window.visibility === Window.Hidden ? albert.clearQueries() : runQuery()
    // }

    function activate(itemIndex, actionIndex) {
        console.info("activate " + itemIndex + " " + actionIndex)
        if (typeof actionIndex === 'undefined')
            actionIndex = 0

        if (sResultsMatches.active)
            albert.currentQuery().activateMatch(itemIndex, actionIndex)
        else
            albert.currentQuery().activateFallback(itemIndex, actionIndex)
        mainWindow.hide()
    }
}
