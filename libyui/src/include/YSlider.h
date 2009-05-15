/*---------------------------------------------------------------------\
|								       |
|		       __   __	  ____ _____ ____		       |
|		       \ \ / /_ _/ ___|_   _|___ \		       |
|			\ V / _` \___ \ | |   __) |		       |
|			 | | (_| |___) || |  / __/		       |
|			 |_|\__,_|____/ |_| |_____|		       |
|								       |
|				core system			       |
|							 (C) SuSE GmbH |
\----------------------------------------------------------------------/

  File:	      YSlider.h

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/

#ifndef YSlider_h
#define YSlider_h

#include "YWidget.h"
#include <ycp/YCPString.h>
#include <ycp/YCPInteger.h>

class YMacroRecorder;

/**
 * @short Implementation of the Slider widget
 */
class YSlider : public YWidget
{
public:

    /**
     * Constructor
     */
    YSlider( const YWidgetOpt &	opt,
	     const YCPString &	label,
	     int 		minValue,
	     int 		maxValue,
	     int 		initialValue );

    /**
     * Returns a descriptive name of this widget class for logging,
     * debugging etc.
     */
    virtual char *widgetClass() { return "YSlider"; }

    /**
     * Implements the ChangeWidget() UI command.
     */
    YCPValue changeWidget( const YCPSymbol & property,
			   const YCPValue &  newValue );

    /**
     * Implements the QueryWidget() UI command.
     */
    YCPValue queryWidget( const YCPSymbol & property );

    /**
     * Change the Slider label.
     *
     * Overwrite this, but call YSlider::setLabel
     * at the end of your own method.
     */
    virtual void setLabel( const YCPString & label );

    /**
     * Change the slider value.
     *
     * Overload this, but call YSlider::setValue()
     * at the end of your own method.
     */
    virtual void setValue( int newValue );


    YCPString	label()		{ return _label; }
    int		value()		{ return _value; }
    int		minValue()	{ return _minValue; }
    int		maxValue()	{ return _maxValue; }


    /**
     * The name of the widget property that holds the keyboard shortcut.
     * Inherited from YWidget.
     */
    const char *shortcutProperty() { return YUIProperty_Label; }

    /**
     * The name of the widget property that will return user input.
     * Inherited from YWidget.
     **/
    const char *userInputProperty() { return YUIProperty_Value; }


private:

    /**
     * Save the widget's user input to a macro recorder.
     * Intentionally declared as "private" so all macro recording internals are
     * handled by the abstract libyui level, not by a specific UI.
     */
    virtual void saveUserInput( YMacroRecorder *macroRecorder );


    YCPString	_label;
    int		_minValue;
    int		_maxValue;
    int		_value;
};


#endif // YSlider_h