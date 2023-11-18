#ifndef COLORLABEL_H
#define COLORLABEL_H

#include <QWidget>
#include <QLabel>
#include <QColor>

class ColorLabel : public QLabel
{
    int tag = -1;
public:
    ColorLabel(int want_tag);
    int get_tag();
};

#endif // COLORLABEL_H
