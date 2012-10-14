var launcherCurrentArugment;
var launcherCurrentArgumentRangeStart;
var launcherBusy = false;

function argumentIsCompleted(arg, pos)
{
    var c = "";

    for (var i = 0; i < pos && i < arg.length; ++ i)
    {
        if (arg.charAt(i) == '"' && c == "")
            c = '"';
        else if (arg.charAt(i) == '"' && c == '"')
            c = ""
        else if (arg.charAt(i) == "'" && c == "")
            c = "'";
        else if (arg.charAt(i) == "'" && c == "'")
            c = "";
    }
    return c == "";
}

function LauncherFinish()
{
    if (launcherBusy) return;
    launcherBusy = true;

    var container = $("#launcher-container");
    var children = container.children(".launcher-argument");
    var result = [];
    for (var i = 0; i < children.size(); ++ i) {
        var str = $(children[i]).text();
        if (str != "")
            result.push($(children[i]).text());
    }

    sys.LauncherSubmit(result.length, result);
    $("body").fadeOut(function() {
        children.remove();
        container.append("<div class='launcher-argument' contenteditable='true'></div>")
    });
}

function LauncherMoveTo(last, current)
{
    if (last.text() != "")
        last.prop("contenteditable", false);
    else if (last.index() != current.index()) last.remove();
    current.prop("contenteditable", true).focus();

    // Selects all text
    var sel, range;
    if (window.getSelection && document.createRange) {
        range = document.createRange();
        range.selectNodeContents(current[0]);
        sel = window.getSelection();
        sel.removeAllRanges();
        sel.addRange(range);
    } else if (document.body.createTextRange) {
        range = document.body.createTextRange();
        range.moveToElementText(current[0]);
        range.select();
   }
}

function LauncherMoveForward(cycle)
{
    var container = $("#launcher-container");
    var current = launcherCurrentArugment;
    var last = current;
    var children = container.children();

    if (current.index() == children.size() - 1) {
        if (cycle) {
            current = $(children[0]);
        } else if (current.text() == "") {
            LauncherFinish();
            return;
        } else container.append(current = $("<div class='launcher-argument'></div>"));
    }
    else current = $(children[current.index() + 1]);

    LauncherMoveTo(last, current);
}

function LauncherMoveBackward(cycle)
{
    var container = $("#launcher-container");
    var current = launcherCurrentArugment;
    var last = current;
    var children = container.children();

    if (current.index() <= 0) {
        if (cycle) { 
            current = $(children[children.size() - 1]);
        } else return;
    } else current = $(children[current.index() - 1]);

    LauncherMoveTo(last, current);
}

function LauncherKeyPressedInCurrentArgument(event)
{
    var bypass = true;
    // Process special key here
    if (event.which == 13) {
        (window.getSelection().anchorNode);
        if (event.ctrlKey) {
            bypass = false;
            // Finish input and send
            LauncherFinish();
        } else if (!event.shiftKey) {
            // As you see you can still input \n by shift+enter
            bypass = false;
            // Switch to next block
            LauncherMoveForward(0);
        }
    } else if (event.which == 37) {
        if (event.ctrlKey && event.altKey) {
            bypass = false;
            LauncherMoveBackward(1);
        }
    } else if (event.which == 39) {
        if (event.ctrlKey && event.altKey) {
            bypass = false;
            LauncherMoveForward(1);
        }
    } else if (event.which == 8) {
        if ($(".launcher-argument[contenteditable=true]").text() == "")
        {
            bypass = false;
            LauncherMoveBackward(0);
        }
    } else if (event.which == 9) {
        bypass = false;
        LauncherMoveForward(1);
    } else if (event.which == 32) {
        if (argumentIsCompleted(launcherCurrentArugment.text(), launcherCurrentArgumentRangeStart))
        {
            bypass = false;
            LauncherMoveForward(0);
        }
        else bypass = true;
    }

    return bypass;
}

$(document).ready(function() {
    $("body").click(function() {
        sys.GetDesktopFocus();
    });

    $(document).bind("keydown", function(event) {
        // To fix the wired focus condition
        var f = window.getSelection().anchorNode;
        launcherCurrentArugment = $(f).closest(".launcher-argument[contenteditable=true]");
        launcherCurrentArgumentRangeStart = window.getSelection().anchorOffset;
        if (launcherCurrentArugment.size() != 0)
            return LauncherKeyPressedInCurrentArgument(event);
    })

    $("body").bind("sysWakeup", function() { 
        $("body").fadeIn(function() {
            sys.GetDesktopFocus();
            $(".launcher-argument[contenteditable=true]").focus();
            launcherBusy = false;
        })
        return false;
    });
});
