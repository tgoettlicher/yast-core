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

  File:		YComboBox.cc

  Author:	Stefan Hundhammer <sh@suse.de>

/-*/


#define YUILogComponent "ui"
#include "YUILog.h"

#include "YUISymbols.h"
#include "YMacroRecorder.h"
#include "YComboBox.h"
#include "YUIException.h"


struct YComboBoxPrivate
{
    YComboBoxPrivate( bool editable )
	: editable( editable )
	, inputMaxLength( -1 )
	{}

    bool	editable;
    string	validChars;
    int		inputMaxLength;
};




YComboBox::YComboBox( YWidget * parent, const string & label, bool editable )
    : YSelectionWidget( parent, label,
			true )		// enforceSingleSelection
    , priv( new YComboBoxPrivate( editable ) )
{
    YUI_CHECK_NEW( priv );
}


YComboBox::~YComboBox()
{
    // NOP
}


bool YComboBox::editable() const
{
    return priv->editable;
}


string YComboBox::validChars()
{
    return priv->validChars;
}


void YComboBox::setValidChars( const string & newValidChars )
{
    priv->validChars= newValidChars;
}


int YComboBox::inputMaxLength() const
{
    return priv->inputMaxLength;
}


void YComboBox::setInputMaxLength( int len )
{
    priv->inputMaxLength = len;
}


string YComboBox::value()
{
    return text();
}


void YComboBox::setValue( const string & newText )
{
    YItem * item = findItem( newText );

    if ( item )
    {
	YSelectionWidget::deselectAllItems();
	item->setSelected();
	setText( item->label() );
    }
    else
    {
	if ( editable() )
	    setText( newText );
	else
	{
	    YUI_THROW( YUIException( "Invalid value" ) );
	}
    }
}


void YComboBox::selectItem( YItem * item, bool selected )
{
    // Check against null pointer and if the item actually belongs to this
    // widget, deselect any previously selected item and select the new one
    YSelectionWidget::selectItem( item, selected );

    if ( selected )
    {
	setText( item->label() );
    }
}


YItem *
YComboBox::selectedItem()
{
    string currentText = text();

    if ( ! currentText.empty() )
    {
	// Try to find an item with this text

	YItem * item = findItem( currentText );

	if ( item )
	{
	    // Make sure exactly this item is selected (and no other)

	    YSelectionWidget::deselectAllItems();
	    item->setSelected( true );

	    return item;
	}
    }
    else
    {
	YSelectionWidget::deselectAllItems();
    }

    return 0;
}


YItemCollection
YComboBox::selectedItems()
{
    YItemCollection selectedItems;

    // There can be no more than one selected item
    YItem * item = selectedItem();

    if ( item )
	selectedItems.push_back( item );

    return selectedItems;
}


const YPropertySet &
YComboBox::propertySet()
{
    static YPropertySet propSet;

    if ( propSet.isEmpty() )
    {
	/*
	 * @property itemID | string  	Value		ID of the selected item or the text the user entered
	 * @property string		Label		caption above the combo box
	 * @property itemList		Items		All items
	 * @property string 		ValidChars	set of valid input characters
	 * @property integer		InputMaxLength	maximum number of input characters
	 * @property string  		IconPath	Base path for icons
	 */
	propSet.add( YProperty( YUIProperty_Value,		YOtherProperty	 ) );
	propSet.add( YProperty( YUIProperty_Items,		YOtherProperty	 ) );
	propSet.add( YProperty( YUIProperty_Label,		YStringProperty	 ) );
	propSet.add( YProperty( YUIProperty_ValidChars,		YStringProperty	 ) );
	propSet.add( YProperty( YUIProperty_InputMaxLength,	YIntegerProperty ) );
	propSet.add( YProperty( YUIProperty_IconPath,		YStringProperty	 ) );
	propSet.add( YWidget::propertySet() );
    }

    return propSet;
}


bool
YComboBox::setProperty( const string & propertyName, const YPropertyValue & val )
{
    propertySet().check( propertyName, val.type() ); // throws exceptions if not found or type mismatch

    if	    ( propertyName == YUIProperty_Value		)	return false; // Need special handling
    else if ( propertyName == YUIProperty_Items 	)	return false; // Needs special handling
    else if ( propertyName == YUIProperty_Label		)	setLabel( val.stringVal() );
    else if ( propertyName == YUIProperty_ValidChars	)	setValidChars( val.stringVal() );
    else if ( propertyName == YUIProperty_InputMaxLength )	setInputMaxLength( val.integerVal() );
    else if ( propertyName == YUIProperty_IconPath 	)	setIconBasePath( val.stringVal() );
    else
    {
	return YWidget::setProperty( propertyName, val );
    }

    return true; // success -- no special processing necessary
}


YPropertyValue
YComboBox::getProperty( const string & propertyName )
{
    propertySet().check( propertyName ); // throws exceptions if not found

    if	    ( propertyName == YUIProperty_Value		)	return YPropertyValue( YOtherProperty );
    else if ( propertyName == YUIProperty_Items 	)	return YPropertyValue( YOtherProperty );
    else if ( propertyName == YUIProperty_Label		)	return YPropertyValue( label() );
    else if ( propertyName == YUIProperty_ValidChars	)	return YPropertyValue( validChars() );
    else if ( propertyName == YUIProperty_InputMaxLength )	return YPropertyValue( inputMaxLength() );
    else if ( propertyName == YUIProperty_IconPath	)	return YPropertyValue( iconBasePath() );
    else
    {
	return YWidget::getProperty( propertyName );
    }
}



void
YComboBox::saveUserInput( YMacroRecorder *macroRecorder )
{
    macroRecorder->recordWidgetProperty( this, YUIProperty_Value );
}

