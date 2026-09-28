// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
Canvas {
    id: symbol
    property string kind: ""
    property color ink: Theme.text
    implicitWidth: 20; implicitHeight: 20
    onKindChanged: requestPaint()
    onInkChanged: requestPaint()
    onWidthChanged: requestPaint()
    onHeightChanged: requestPaint()
    onPaint: {
        const c=getContext("2d")
        c.reset(); c.scale(width/24,height/24)
        c.strokeStyle=ink; c.fillStyle=ink; c.lineWidth=1.8; c.lineJoin="round"; c.lineCap="round"
        c.beginPath()
        if(kind==="save") { c.rect(4,3,16,18); c.rect(8,3,8,6); c.rect(8,14,8,7) }
        else if(kind==="folder") { c.moveTo(3,20);c.lineTo(3,5);c.lineTo(10,5);c.lineTo(12,8);c.lineTo(21,8);c.lineTo(21,20);c.closePath() }
        else if(kind==="build") { c.moveTo(7,3);c.lineTo(20,12);c.lineTo(7,21);c.closePath() }
        else if(kind==="help") { c.arc(12,12,9,0,Math.PI*2);c.stroke();c.beginPath();c.moveTo(9,8);c.bezierCurveTo(9,4,17,5,15,10);c.lineTo(12,13);c.lineTo(12,14);c.moveTo(12,18);c.lineTo(12,18.2) }
        else if(kind==="more") { for(let x=5;x<=19;x+=7){c.moveTo(x+1,12);c.arc(x,12,1,0,Math.PI*2)} }
        c.stroke()
    }
}
