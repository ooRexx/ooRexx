

                           Resizable Dialogs
                           =================


There are 2 basic ways to produce resizable dialogs in ooDialog.

One way is to use the DlgArea and its subclass, DlgAreaU.  Note that the
DlgArea class in itself does not produce a resizable dialog.  It is the
DlgAreaU subclass that effectively allows resizing.

The other way is to have a dialog class inherit the ResizingAdmin
subclass.

This subdirectory tree contains two subdirectories that contain example
programs showing how to use both methods.  Not surprisingly, the
subdirectory, DlgAreaU contains examples using the DlgAreaU method and
the subdirectory, ResizingAdmin, contains examples showing how to use
the ResizingAdmin mixin class to produce resizable dialogs.
