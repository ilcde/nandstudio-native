# Android popup access regression

The first safe-area change covered the header buttons only. The user's next
screenshot showed the Files popup centered on the short header, with its first
action under the status bar and a hover tooltip obscuring other actions.

Version 0.1.1 uses explicit popup coordinates in the window overlay, anchored
below the pressed button with a top margin equal to the header height. Files
and More use item popups so these bounds are honored consistently. Touch platforms
disable button hover tooltips. The app version appears in More; Android's version
code is incremented to 2. Debug signing keys still differ between CI builds.

Desktop regression checks click the Files button with a simulated 48-unit top
inset, assert that Open workspace is below the header, click it, and verify the
folder chooser opens. More bounds are also checked. Android's release test uses
actual adb touch events at measured control positions, opens both menus and the
workspace chooser, and captures screenshots. A toolbar-only report is rejected
by release staging. Actual provider selection/import/editing remains a separate
unfinished workflow; the test does not label a displayed chooser as an opened
editable workspace.
