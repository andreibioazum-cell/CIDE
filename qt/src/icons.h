#pragma once

#include <QColor>
#include <QIcon>
#include <QString>

/*
 * Material-style icons drawn with QPainter (24x24 grid), matching the
 * icon names used by the web interface at commit 3673f5a
 * (res/icons + res/file-icons fonts).
 */
namespace Icons {

enum Icon {
    Menu,            /* hamburger (toggle-sidebar)        */
    MoreVert,        /* ⋮ (toggle-menu)                   */
    Edit,            /* pencil (toggle-edit-menu)         */
    PlayArrow,       /* run                               */
    Save,            /* save / save as                    */
    Folder,          /* files                             */
    FolderOpen,      /* open folder                       */
    Close,           /* clearclose                        */
    History,         /* historyrestore (open recent)      */
    Search,          /* search / find file                */
    Code,            /* console                           */
    Terminal,        /* terminal                          */
    Apps,            /* running processes                 */
    Settings,        /* settings                          */
    Help,            /* help                              */
    Exit,            /* logout / exit                     */
    Documents,       /* files sidebar app                 */
    Extension,       /* plugins                           */
    Notifications,   /* notifications sidebar app         */
    Undo,
    Redo,
    KeyboardTab,
    ArrowUp,
    ArrowDown,
    ArrowLeft,
    ArrowRight,
    MoveLineUp,
    MoveLineDown,
    CopyLineUp,
    CopyLineDown,
    Paste,
    TextFormat,      /* select all                        */
    Add,             /* new file                          */
    DocText,         /* open file                         */
    Tune,            /* app settings                      */
    Info,            /* file properties / about           */
    Pin,             /* pin tab                           */
    LastPage,        /* close tabs to right               */
    FirstPage,       /* close tabs to left                */
    CompareArrows,   /* close other tabs                  */
    SubdirArrowLeft, /* goto line                         */
    Palette,         /* insert color                      */
    Share,
    OpenInBrowser,   /* open with                         */
    Home,            /* add to home screen                */
    ChevronUp,
    ChevronDown,
    Globe,           /* website                           */
    CommandPalette,
    ControlKey,      /* keyboard_control                  */
    FileGlyph,       /* generic file glyph for the tree   */
};

/* Returns a cached icon of the given kind painted in `color`. */
QIcon icon(Icon kind, const QColor &color = QColor(245, 245, 245));

/* Generic file icon, colored like the web file-icons font by extension. */
QIcon fileIcon(const QString &fileName, bool isDir);
QColor fileIconColor(const QString &fileName);

} // namespace Icons
