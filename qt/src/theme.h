#pragma once

#include <QColor>
#include <QString>

/*
 * Colors of the locked "Dark" theme from CIDE commit 3673f5a
 * (src/theme/preInstalled.js). The editor uses the "one_dark"
 * CodeMirror theme colors.
 */
namespace Theme {

inline const QColor Primary{35, 39, 42};          /* rgb(35, 39, 42)  #232729 */
inline const QColor PrimaryText{245, 245, 245};   /* rgb(245,245,245) #f5f5f5 */
inline const QColor Secondary{45, 49, 52};        /* rgb(45, 49, 52)  #2d3134 */
inline const QColor SecondaryText{228, 228, 228}; /* rgb(228,228,228) #e4e4e4 */
inline const QColor Active{66, 133, 244};         /* rgb(66,133,244)  #4285f4 */
inline const QColor ActiveText{255, 255, 255};
inline const QColor Link{138, 180, 248};          /* rgb(138,180,248) #8ab4f8 */
inline const QColor Error{255, 185, 92};          /* rgb(255,185, 92) #ffb95c */
inline const QColor Danger{220, 38, 38};          /* rgb(220, 38, 38) #dc2626 */
inline const QColor Notice{255, 218, 12};         /* #ffda0c unsaved marker */

inline QColor borderColor() { return QColor(188, 188, 188, 38); } /* 0.15 alpha */
inline QColor scrollbarColor() { return QColor(255, 255, 255, 51); }
inline QColor railBackground() { return QColor(0, 0, 0, 38); }        /* rgba(0,0,0,0.15)  */
inline QColor railActive() { return QColor(255, 255, 255, 31); }      /* rgba(255,255,255,.12) */
inline QColor railHover() { return QColor(255, 255, 255, 20); }       /* rgba(255,255,255,.08) */
inline QColor tabActiveBackground() { return QColor(0, 0, 0, 51); }   /* rgba(0,0,0,0.2) */

/* CodeMirror "one_dark" (src/theme/preInstalled.js preferredEditorTheme) */
inline const QColor EditorBackground{40, 44, 52};   /* #282c34 */
inline const QColor EditorForeground{171, 178, 191};/* #abb2bf */
inline const QColor EditorGutter{73, 81, 98};       /* #495162 */
inline const QColor EditorSelection{62, 68, 81};    /* #3e4451 */
inline const QColor EditorCursor{82, 139, 255};     /* #528bff */
inline const QColor EditorActiveLine{44, 49, 60};   /* #2c313c */
inline const QColor EditorComment{127, 132, 142};   /* #7f848e */
inline const QColor EditorKeyword{198, 120, 221};   /* #c678dd */
inline const QColor EditorString{152, 195, 121};    /* #98c379 */
inline const QColor EditorNumber{209, 154, 102};    /* #d19a66 */
inline const QColor EditorFunction{97, 175, 239};   /* #61afef */
inline const QColor EditorVariable{224, 108, 117};  /* #e06c75 */
inline const QColor EditorOperator{86, 182, 194};   /* #56b6c2 */
inline const QColor EditorTag{224, 108, 117};       /* #e06c75 */
inline const QColor EditorAttribute{209, 154, 102}; /* #d19a66 */
inline const QColor EditorMeta{97, 175, 239};       /* #61afef */

/* Applies the dark palette + widget stylesheet to the application. */
void apply();

} // namespace Theme
