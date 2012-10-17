var launcherCurrentArgument;
var launcherCurrentArgumentRangeStart;
var launcherBusy = false;

function ArgumentIsCompleted(arg, pos)
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

function LauncherDoCompletion()
{
    var container = $("#launcher-completion-container");
    container.empty();
    var comp;
    if (launcherCurrentArgument.index() == 0)
        comp = sys.CompleteProg(launcherCurrentArgument.text());
    else comp = sys.CompleteFileName(launcherCurrentArgument.text());
    if (comp != null)
    {
        var dcomp = "";
        var s = comp[0];
        var e = comp[comp.length - 1];
        var i = 0;
        while (i < s.length && i < e.length && s.charAt(i) == e.charAt(i))
        {
            dcomp += s.charAt(i);
            ++ i;
        }

        for (i = 0; i < comp.length; ++ i)
        {
            container.append("<div class='launcher-completion'>" + comp[i] + "</div>");
        }

        launcherCurrentArgument.text(dcomp);
        // Move the cursor to the end
        var dom = launcherCurrentArgument.get(0);
        if (document.createRange)
        {
            range = document.createRange();
            range.selectNodeContents(dom);
            range.collapse(false);
            selection = window.getSelection();
            selection.removeAllRanges();
            selection.addRange(range);
        }
        else if (document.selection)
        { 
            range = document.body.createTextRange();
            range.moveToElementText(dom);
            range.collapse(false);
            range.select();
        }
    }
}

function LauncherClean()
{
    var container = $("#launcher-container");
    container.empty();
    container.append("<div class='launcher-argument' contenteditable='true'></div>");

    var compContainer = $("#launcher-completion-container");
    compContainer.empty();
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
    sys.HideAndReset();
    LauncherClean();
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
    var current = launcherCurrentArgument;
    var last = current;
    var children = container.children();

    if (current.index() == children.size() - 1) {
        if (cycle) {
            current = $(children[0]);
        } else if (current.text() == "") {
            return;
        } else container.append(current = $("<div class='launcher-argument'></div>"));
    }
    else current = $(children[current.index() + 1]);

    LauncherMoveTo(last, current);
}

function LauncherMoveBackward(cycle)
{
    var container = $("#launcher-container");
    var current = launcherCurrentArgument;
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
        // enter
        (window.getSelection().anchorNode);
        if (event.ctrlKey) {
            bypass = false;
            // TODO behavior
        } else if (!event.shiftKey) {
            // As you see you can still input \n by shift+enter
            bypass = false;
            // Finish input and send
            LauncherFinish();
        }
    } else if (event.which == 37) {
        // left
        if (event.ctrlKey && event.altKey) {
            bypass = false;
            LauncherMoveBackward(1);
        }
    } else if (event.which == 39) {
        // right
        if (event.ctrlKey && event.altKey) {
            bypass = false;
            LauncherMoveForward(1);
        }
    } else if (event.which == 8) {
        // backspace
        if ($(".launcher-argument[contenteditable=true]").text() == "")
        {
            bypass = false;
            LauncherMoveBackward(0);
        }
    } else if (event.which == 9) {
        // tab
        bypass = false;
        LauncherDoCompletion();
    } else if (event.which == 32) {
        // space
        if (ArgumentIsCompleted(launcherCurrentArgument.text(), launcherCurrentArgumentRangeStart))
        {
            bypass = false;
            LauncherMoveForward(0);
        }
        else bypass = true;
    }

    return bypass;
}

$(document).ready(function() {
    if (!sys.LoadPlugin("launcher", "weblet-plugin-launcher.so"))
    {
        sys.DebugPrint("Cannot load the launcher plugin");
        sys.Exit();
    }

    if (!sys.LoadPlugin("completion", "weblet-plugin-completion.so"))
    {
        sys.DebugPrint("Cannot load the completion plugin");
        sys.Exit();
    }

    $("body").click(function() {
        sys.GetDesktopFocus();
    });

    $(document).bind("keydown", function(event) {
        if (event.which == 27)
        {
            // ESC
            launcherBusy = true;
            sys.HideAndReset();
            LauncherClean();
            return false;
        }
        // To fix the wired focus condition
        var f = window.getSelection().anchorNode;
        launcherCurrentArgument = $(f).closest(".launcher-argument[contenteditable=true]");
        if (launcherCurrentArgument.size() != 0)
        {
            launcherCurrentArgumentRangeStart = window.getSelection().anchorOffset;
            return LauncherKeyPressedInCurrentArgument(event);
        }

        return true;
    })

    $("body").bind("sysWakeup", function() { 
        sys.DebugPrint("sysWakeup");
        sys.Show();
        sys.GetDesktopFocus();
        $(".launcher-argument[contenteditable=true]").focus();
        launcherBusy = false;
        return false;
    });
});
