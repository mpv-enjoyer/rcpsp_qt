#include "colorlabel.h"

ColorLabel::ColorLabel(int want_tag)
{
    tag = want_tag;
}

int ColorLabel::get_tag()
{
    return tag;
}
