import Qt 4.6

QtObject {
    property bool test: false

    Component.onCompleted: {
        var x = new XMLHttpRequest;

        x.open("GET", "testdocument.html");
        x.send();

        try {
            x.getAllResponseHeaders();
        } catch (e) {
            if (e.code == DOMException.INVALID_STATE_ERR)
                test = true;
        }
    }
}
