// Copyright (c) 2023 Manuel Schneider

function testKey(event, key) {
    if (event.key === key){
        event.accepted = true
//        console.warn("accepted")
        return true
    }
    return false
}

function testAnyKey(event, keyArray) {
    for (const key of keyArray)
        if (testKey(event, key))
            return true;
    return false;
}

function testModifiers(event, modifiers) {
    return (event.modifiers & modifiers) === modifiers
}

function testKeyCombination(event, modifiers, key) {
    return testModifiers(event, modifiers) && testKey(event, key)
}

function testAnyKeyCombination(event, modifiers, keyArray) {
    return testModifiers(event, modifiers) && testAnyKey(event, keyArray)
}

function printKeyPress(name, event) {
    let s = albert.kcString(event.modifiers|event.key)
    console.warn("QML PRESS "+s+" "+name)
}

function printKeyRelease(name, event) {
    let s = albert.kcString(event.modifiers|event.key)
    console.warn("QML RELEASE "+s+" "+name)
}

////    function loadJavaScriptFile(url) {
////        var request = new XMLHttpRequest();
////        request.open("GET", url, true);
////        request.onreadystatechange = function() {
////            if (request.readyState === XMLHttpRequest.DONE) {
////                if (request.status === 200) {
////                    // File loaded successfully
////                    var scriptCode = request.responseText;
////                    Qt.createQmlObject(scriptCode, parentItem);
////                } else {
////                    console.error("Failed to load JavaScript file: " + request.status);
////                }
////            }
////        };
////        request.send();
////    }
////    Component.onCompleted: {
////        fetch(":themes.js")
////        loadJavaScriptFile(":themes.js")
////    }
