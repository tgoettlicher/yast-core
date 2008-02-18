/*---------------------------------------------------------------------\
|                                                                      |  
|                      __   __    ____ _____ ____                      |  
|                      \ \ / /_ _/ ___|_   _|___ \                     |  
|                       \ V / _` \___ \ | |   __) |                    |  
|                        | | (_| |___) || |  / __/                     |  
|                        |_|\__,_|____/ |_| |_____|                    |  
|                                                                      |  
|                               core system                            | 
|                                                        (C) SuSE GmbH |  
\----------------------------------------------------------------------/ 

   File:       Y2CCUI.h

   Author:     Stanislav Visnovsky <visnov@suse.de>
   Maintainer: Stanislav Visnovsky <visnov@suse.de>

/-*/
// -*- c++ -*-

/*
 * Component Creator that executes access to UI
 *
 * Author:     Stanislav Visnovsky <visnov@suse.de>
 */

#ifndef Y2CCUI_h
#define Y2CCUI_h

#include <y2/Y2ComponentCreator.h>

class Y2Component;


/**
 * Component creator that provides access to the UI
 **/
class Y2CCUI : public Y2ComponentCreator
{
    
public:
    /**
     * Constructor.
     **/
    Y2CCUI();

    virtual bool isServerCreator () const { return true; }

    /**
     * Creation function for Y2Components.
     *
     * This class can create Y2UIComponents named:
     *     - "UI"	(generic UI, for testsuites)
     *     - "ui"	(automatically choose a suitable UI)
     *     - "qt"	(Qt UI)
     *     - "ncurses"	(NCurses (text based) UI)
     *
     * In future versions also:
     *
     *     - "gtk"	(Gtk UI)
     *
     * For all other names, 0 is returned, so the Y2ComponentBroker will keep
     * on trying with other available creators. 
     **/
    virtual  Y2Component * create( const char * name ) const;

    /**
     * Creation function for non-built-in components.
     **/
    virtual Y2Component * createInLevel( const char * name, int level, int currentLevel ) const;
    
    /**
     * Name space provider function.
     * This class can provide the "UI" name space.
     *
     * For all other names, 0 is returned, so the Y2ComponentBroker will keep
     * on trying with other available creators. 
     **/
    virtual  Y2Component * provideNamespace( const char * name );
};


#endif // Y2CCUI_h
