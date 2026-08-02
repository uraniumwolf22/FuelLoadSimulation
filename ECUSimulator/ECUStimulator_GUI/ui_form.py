# -*- coding: utf-8 -*-

################################################################################
## Form generated from reading UI file 'form.ui'
##
## Created by: Qt User Interface Compiler version 6.11.1
##
## WARNING! All changes made in this file will be lost when recompiling UI file!
################################################################################

from PySide6.QtCore import (QCoreApplication, QDate, QDateTime, QLocale,
    QMetaObject, QObject, QPoint, QRect,
    QSize, QTime, QUrl, Qt)
from PySide6.QtGui import (QBrush, QColor, QConicalGradient, QCursor,
    QFont, QFontDatabase, QGradient, QIcon,
    QImage, QKeySequence, QLinearGradient, QPainter,
    QPalette, QPixmap, QRadialGradient, QTransform)
from PySide6.QtWidgets import (QApplication, QSizePolicy, QSlider, QWidget)

class Ui_ECUGUI(object):
    def setupUi(self, ECUGUI):
        if not ECUGUI.objectName():
            ECUGUI.setObjectName(u"ECUGUI")
        ECUGUI.resize(800, 600)
        self.coolantSlider = QSlider(ECUGUI)
        self.coolantSlider.setObjectName(u"coolantSlider")
        self.coolantSlider.setGeometry(QRect(180, 190, 16, 160))
        self.coolantSlider.setOrientation(Qt.Orientation.Vertical)

        self.retranslateUi(ECUGUI)

        QMetaObject.connectSlotsByName(ECUGUI)
    # setupUi

    def retranslateUi(self, ECUGUI):
        ECUGUI.setWindowTitle(QCoreApplication.translate("ECUGUI", u"ECUGUI", None))
    # retranslateUi

