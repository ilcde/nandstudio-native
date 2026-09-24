// SPDX-License-Identifier: GPL-3.0-or-later
pragma Singleton
import QtQuick
QtObject {
    property bool dark: true
    property real scale: 1
    readonly property int xs: Math.round(4 * scale)
    readonly property int sm: Math.round(8 * scale)
    readonly property int md: Math.round(16 * scale)
    readonly property int lg: Math.round(24 * scale)
    readonly property int radius: 8
    readonly property int controlHeight: Math.round(44 * scale)
    readonly property int bodySize: Math.round(14 * scale)
    readonly property color canvas: dark ? "#101722" : "#edf1f6"
    readonly property color surface: dark ? "#182231" : "#ffffff"
    readonly property color raised: dark ? "#233247" : "#e4ebf4"
    readonly property color border: dark ? "#40516a" : "#aab9cc"
    readonly property color text: dark ? "#edf3fc" : "#162539"
    readonly property color muted: dark ? "#aabbcf" : "#526781"
    readonly property color accent: dark ? "#80b7ff" : "#245da1"
    readonly property color selection: dark ? "#304d72" : "#cde1fa"
    readonly property color danger: dark ? "#ffa7a7" : "#9f2323"
    readonly property color currentLine: dark ? "#1e2b3d" : "#edf4ff"
}
