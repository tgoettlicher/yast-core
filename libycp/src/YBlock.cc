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

   File:	YBlock.cc

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

/-*/
// -*- c++ -*-

#include "ycp/YBlock.h"
#include "ycp/YCPVoid.h"

#include <stack>

#ifndef DO_DEBUG
#define DO_DEBUG 0
#endif

// needed for YBlock

#include "ycp/SymbolEntry.h"
#include "ycp/SymbolTable.h"
#include "ycp/YExpression.h"

#include "ycp/Bytecode.h"
#include "ycp/Scanner.h"

#include "ycp/y2log.h"
#include "ExecutionEnvironment.h"

extern ExecutionEnvironment ee;

// ------------------------------------------------------------------

IMPL_DERIVED_POINTER(YBlock, YCode);

// ------------------------------------------------------------------
// block (-> list of statements, list of symbols, kind)

YBlock::YBlock (const std::string & filename, YBlock::blockkind_t kind)
    : YCode (yeBlock)
    , m_kind (kind)
    , m_name ("")
    , m_tenvironment (0)
    , m_last_tparm (0)
    , m_point (0)
    , m_statements (0)
    , m_last_statement (0)
{
#if DO_DEBUG
    y2debug ("YBlock::YBlock [%p] (%s)", this, filename.c_str());
#endif
    // add filename as SymbolEntry:c_filename
    SymbolEntryPtr sentry = new SymbolEntry (nameSpace(), 0, filename.c_str(), SymbolEntry::c_filename, Type::Unspec);
    addSymbol (sentry);

    m_point = new Point (sentry);
}


// intermediate block
YBlock::YBlock (const Point *point)
    : YCode (yeBlock)
    , m_kind (b_unknown)
    , m_name ("")
    , m_tenvironment (0)
    , m_last_tparm (0)
    , m_point (point)
    , m_statements (0)
    , m_last_statement (0)
{
}


YBlock::~YBlock ()
{
#if DO_DEBUG
    y2debug ("YBlock::~YBlock [%p]", this);
#endif
    stmtlist_t *stmt = m_statements;
    while (stmt)
    {
	stmtlist_t *nexts = stmt->next;
	delete stmt;
	stmt = nexts;
    }

    if (m_kind == b_file)			// point belongs to file kind block
    {
	if (m_point)
	    delete m_point;
    }

    yTElist_t *tp = m_tenvironment;
    while (tp)
    {
	yTElist_t *next = tp->next;
	delete tp;
	tp = next;
    }
}


const std::string 
YBlock::filename () const
{
    return m_point->filename();
}


// add new value code to this block
//   (used for functions which accept either symbolic variables or values, e.g. foreach())
// returns position
unsigned int
YBlock::newValue (constTypePtr type, YCodePtr code)
{
    static char name[8];
    sprintf (name, "_%d", symbolCount());		// create 'fake' name

#if DO_DEBUG
    y2debug ("YBlock::newValue (%s %s)", type->toString().c_str(), name);
#endif

// FIXME: check duplicates
    return addSymbol (new SymbolEntry (nameSpace(), symbolCount(), name, SymbolEntry::c_const, type, code));
}


// SymbolTable for global module environment (m_kind == b_module)
SymbolTable *
YBlock::table () const
{
#if DO_DEBUG
    y2debug ("YBlock[%p]::table() -> %p", this, m_table );
#endif
    return m_table;
}


// add new table entry to this block
//  and attach it to m_tenvironment
//   return NULL if symbol of same name already declared in this block
TableEntry *
YBlock::newEntry (const char *name, SymbolEntry::category_t cat, constTypePtr type, unsigned int line)
{
#if DO_DEBUG
    y2debug ("YBlock(%p, ns %p)::newEntry (%s, cat %d, type %s, line %d)", this, nameSpace(), name, (int)cat, type->toString().c_str(), line);
#endif

    // check duplicates (in Y2Namespace)
    if (lookupSymbol (name) != 0)
    {
#if DO_DEBUG
    y2debug ("already defined");
#endif
	return 0;
    }

    SymbolEntryPtr sentry = new SymbolEntry (nameSpace(), symbolCount(), name, cat, type);
    addSymbol (sentry);

    // cat == SymbolEntry::c_filename: inclusion point, include file sentry included at line in (current file) m_point
    // definition point (sentry is defined in (current file) m_point at line)

    Point *point = new Point (sentry, line, m_point);
    if (cat == SymbolEntry::c_filename)
    {
	m_point = point;
    }
    TableEntry *tentry = new TableEntry (sentry->name(), sentry, point);	// link symbol and declaration point
    attachEntry (tentry);

    return tentry;
}


// Attach entry (variable, typedef, ...) to local environment
void
YBlock::attachEntry (TableEntry *tentry)
{
#if DO_DEBUG
    y2debug ("YBlock[%p]::attachEntry (%p)", this, tentry/*->toString().c_str()*/);
#endif

    yTElist_t *newt = new (yTElist_t);
    newt->tentry = tentry;
    newt->next = 0;

    if (m_tenvironment == 0)
    {
	m_tenvironment = newt;
	m_last_tparm = m_tenvironment;
    }
    else if (m_last_tparm == 0)
    {
	y2error ("YBlock::attachEntry after detach_environemt !");
    }
    else
    {
	m_last_tparm->next = newt;
	m_last_tparm = newt;
    }
    return;
}


void
YBlock::attachStatement (YStatementPtr statement)
{
#if DO_DEBUG
    y2debug ("YBlock[%p]::attachStatement (%s)", this, statement ? statement->toString().c_str() : "<NULL>");
#endif
    if (statement == 0)
    {
	return;
    }
    stmtlist_t *newstmt = new (stmtlist_t);
    newstmt->stmt = statement;
    newstmt->next = 0;
    if (m_statements == 0)
    {
	m_statements = newstmt;
	m_last_statement = m_statements;
    }
    else
    {
	m_last_statement->next = newstmt;
        m_last_statement = newstmt;
    }
    
    return;
}


void
YBlock::pretachStatement (YStatementPtr statement)
{
#if DO_DEBUG
    y2debug ("YBlock[%p]::pretachStatement (%s)", this, statement->toString().c_str());
#endif

    stmtlist_t *newstmt = new (stmtlist_t);
    newstmt->stmt = statement;
    newstmt->next = m_statements;
    m_statements = newstmt;

    return;
}


// bind namespace entry
TableEntry *
YBlock::newNamespace (const string & name, Y2Namespace *name_space, int line)
{
#if DO_DEBUG
    y2debug ("YBlock[%p]::newNamespace ('%s' namespace@<%p>):%d", this, name.c_str(), name_space, line);
#endif

    TableEntry *tentry = newEntry (Scanner::doStrdup (name.c_str()), SymbolEntry::c_module, Type::Unspec, line);
    tentry->sentry()->setPayloadNamespace (name_space);

#if DO_DEBUG
    y2debug ("environment of %p after addNamespace (%s)", this, environmentToString().c_str());
#endif
    return tentry;
}


// detach environment from symbol table
//   remove all in m_tenvironment listed table entries

void
YBlock::detachEnvironment (SymbolTable *table)
{
#if DO_DEBUG
    y2debug ("YBlock::detachEnvironment of %p (%s) from %s", this, environmentToString().c_str(), table->toString().c_str());
#endif

    // unlink table entries belonging to table (usually, these are the local symbols)

    yTElist_t *tp = m_tenvironment;
    yTElist_t *prev = 0;
    while (tp)
    {
	yTElist_t *next = tp->next;

	if (tp->tentry
	    && tp->tentry->table() == table)
	{
#if DO_DEBUG
	    y2debug ("Remove %s", tp->tentry->toString().c_str());
#endif
	    table->remove (tp->tentry);			// remove the TableEntry
	    delete tp;
	    if (prev != 0)
	    {
		prev->next = next;
	    }
	    else
	    {
		m_tenvironment = next;
	    }
	}
	else
	{
	    prev = tp;
	}
	tp = next;
    }

    if (prev == 0)			// all removed
    {
	m_tenvironment = 0;
    }
    m_last_tparm = 0;

    return;
}


void
YBlock::setKind (YBlock::blockkind_t kind)
{
#if DO_DEBUG
    y2debug ("YBlock::setKind %p: %d", this, (int)kind);
#endif
    m_kind = kind;
    return;
}


YBlock::blockkind_t
YBlock::kind () const
{
    return m_kind;
}


YSReturnPtr
YBlock::justReturn () const
{
    if (m_statements != 0)
    {
	YStatementPtr stmt = m_statements->stmt;
	if (stmt->kind() == YCode::ysReturn)
	{
	    return (YSReturnPtr)stmt;
	}
    }
    return 0;
}


const string 
YBlock::name () const
{
    return m_name;
}


void
YBlock::setName (const string &name)
{
    m_name = name;
    return;
}


const Point *
YBlock::point () const
{
    return m_point;
}


void
YBlock::endInclude ()
{
    const Point *point = m_point->point();
    if (point == 0)
    {
	y2error ("YBlock::endInclude() with empty chain (%s)", m_point->toString().c_str());
    }
    else
    {
#if DO_DEBUG
	y2debug ("YBlock::endInclude(%s)", m_point->toString().c_str());
#endif
	m_point = point;
    }
    return;
}


string
YBlock::environmentToString () const
{
    string s;

    yTElist_t *tp = m_tenvironment;
    while (tp)
    {
	if (!tp->tentry->sentry()->isFilename())
	{
	    s += "\n    //T: ";
	    s += tp->tentry->toString();
	}
	tp = tp->next;
    }

    s += symbolsToString();

    return s;
}


string
YBlock::toString () const
{
    string s;
    if (m_kind == b_using)
    {
	s = (m_name + "::");
    }
    s += "{";

    if (isModule())
    {
	s += "\n    module \"" + m_name + "\";\n";
    }

    s += environmentToString ();
    if (isModule()
	|| isFile())
    {
	s += "\n    // filename: \"" + filename() + "\"";
    }

    stmtlist_t *stmt = m_statements;
    while (stmt)
    {
	s += "\n    ";
	s += stmt->stmt->toString();
	stmt = stmt->next;
    }

    s += "\n}\n";
    return s;
}


YCPValue
YBlock::evaluate (bool cse)
{
    if (cse)
    {
	return YCPNull();
    }

#if DO_DEBUG
    y2debug ("YBlock::evaluate([%d]%s)\n", (int)m_kind, toString().c_str());
#endif

    string restore_name;
    if (!filename().empty())
    {
	restore_name = ee.filename ();
	ee.setFilename (filename());
    }

    stmtlist_t *stmt = m_statements;
    YCPValue value = YCPVoid ();
    while (stmt)
    {
	YStatementPtr statement = stmt->stmt;
	
#if DO_DEBUG
	y2debug ("%d: %s", statement->line (), statement->toString ().c_str ());
#endif

	ee.setStatement (statement);
	value = statement->evaluate ();

	if (!value.isNull())
	{
#if DO_DEBUG
	    y2debug ("Block exit (%s)", value->toString().c_str());
#endif
	    break;
	}

	stmt = stmt->next;
    }
    if (!restore_name.empty())
    {
	ee.setFilename (restore_name);
    }

#if DO_DEBUG
    y2debug ("YBlock::evaluate done (stmt %p, kind %d, value '%s')\n", stmt, m_kind, value.isNull() ? "NULL" : value->toString().c_str());
#endif

    // if stmt==0 we're at the end of the block. If the block is evaluated as a statement,
    //   it returns NULL, else it returns Void
    if (stmt == 0)
    {
	if (m_kind == b_statement)
	{
	    return YCPNull();
	}
	return YCPVoid();
    }

    // if stmt!=0 we just evaluated a break or return statement. A 'return;' evaluates to YCPReturn
    return value;
}


YCPValue
YBlock::evaluate (int statement_index, bool skip_initial_imports)
{
#if DO_DEBUG
    y2debug("YBlock::evaluate(#%d)\n", statement_index);
#endif
    
    stmtlist_t *stmt = m_statements;
    YCPValue value = YCPVoid ();

    while (skip_initial_imports
	   && stmt
	   && stmt->stmt->kind() == YCode::ysImport)
    {
	stmt =stmt->next;
    }
    
    while (stmt && statement_index > 0)
    {
	stmt = stmt->next;
	statement_index--;
    }

    if (!stmt)
    {
	// we are at the end
	return YCPNull ();
    }
    
    y2milestone("YBlock::evaluating:\n%s", stmt->stmt->toString ().c_str());

    value = stmt->stmt->evaluate ();
    
#if DO_DEBUG
    y2debug("YBlock::evaluate statement done (value '%s')\n", value.isNull() ? "NULL" : value->toString().c_str());
#endif

    return value;
}


int
YBlock::statementCount () const
{
    int res = 0;
    stmtlist_t *stmt = m_statements;

    while (stmt)
    {
	stmt = stmt->next;
	res++;
    }
    
    return res;
}


// --------------------------
// bytecode I/O

YBlock::YBlock (std::istream & str)
    : YCode (yeBlock)
    , m_kind (b_unknown)
    , m_name ("")
    , m_tenvironment (0)
    , m_last_tparm (0)
    , m_point (0)
    , m_statements (0)
    , m_last_statement (0)
{
    Bytecode::readString (str, m_name);		// read name

    m_kind = (blockkind_t)Bytecode::readInt32 (str);

    Bytecode::pushNamespace (nameSpace());

    unsigned int scount = Bytecode::readInt32 (str);	// read Y2Namespace::m_symbols

#if DO_DEBUG
    y2debug("YBlock::fromStream (%p:\"%s\", %d entries, kind %d)", this, m_name.c_str(), scount, m_kind);
#endif

    // read all symbol entries belonging to this block

    if (scount > 0)
    {
	for (unsigned int i = 0; i < scount; i++)
	{
	    SymbolEntryPtr sentry = new SymbolEntry (str, nameSpace());
	    addSymbol (sentry);
	}

	if (m_kind == b_module)			// if its a module, re-construct the table
	{
	    int tcount = Bytecode::readInt32 (str);
#if DO_DEBUG
	    y2debug ("Module with %d table entries", tcount);
#endif

	    if (tcount > 0
		&& m_table == 0)
	    {
		m_table = new SymbolTable (-1);
	    }

	    // HACK ahead: Y2ALLGLOBAL should make all
	    // symbols visible. It works now, but as a trade-off,
	    // line numbers are lost also for globals.
	    while (tcount-- > 0)
	    {
		if (getenv("Y2ALLGLOBAL") != NULL)
		{
		    TableEntry t(str);				// FIXME: this object is temporary and unused
		}
		else
		{
		    TableEntry *tentry = new TableEntry (str);
		    attachEntry (tentry);
		    m_table->enter (tentry);
		}
	    }

	    if (getenv("Y2ALLGLOBAL") != NULL)
	    {
		delete m_table; // FIXME: memory leak
		m_table = new SymbolTable(-1);
		m_table->openXRefs ();
		for (unsigned int i = 0 ; i < scount ; i++)
		{
		    SymbolEntryPtr sentry = symbolEntry (i);
		    if (!sentry->isModule()				// don't re-export imported modules
			&& !sentry->isNamespace())			//   or predefined namespaces
		    {
			TableEntry* tentry = m_table->enter(sentry->name (), sentry, 0);
			attachEntry (tentry);
		    }
		}
	    }
	}
    }

    m_point = new Point (str);

    u_int32_t count = Bytecode::readInt32 (str);
#if DO_DEBUG
    y2debug ("YBlock::fromStream (%p:\"%s\", %d statements)", this, m_name.c_str(), count);
#endif

    stmtlist_t *stmt;
    for (u_int32_t i = 0; i < count; i++)
    {
	stmt = new stmtlist_t;

	YCodePtr code = Bytecode::readCode (str);
	if (code == 0)
	{
	    m_kind = b_unknown;
	    m_valid = false;
	    break;
	}
	if (!code->isStatement())			// code must be statement
	{
	    y2error ("Code is no statement: %d", code->kind());
	    m_valid = false;
	    break;
	}

	stmt->stmt = (YStatementPtr)code;
	stmt->next = 0;
	if (stmt->stmt == 0)
	{
	    break;
	}
	if (m_statements == 0)
	{
	    m_statements = stmt;
	}
	else
	{
	    m_last_statement->next = stmt;
	}
	m_last_statement = stmt;
    }

    Bytecode::popUptoNamespace (this);
#if DO_DEBUG
    y2debug ("done");
#endif
}


std::ostream &
YBlock::toStream (std::ostream & str) const
{
#if DO_DEBUG
    y2debug ("YBlock::toStream (%p:\"%s\", kind %d)", this, m_name.c_str(), m_kind);
#endif
    YCode::toStream (str);

    Bytecode::pushNamespace (nameSpace());

    Bytecode::writeString (str, m_name);			// write name
    Bytecode::writeInt32 (str, m_kind);				// write kind

#if DO_DEBUG
    y2debug ("YBlock %p: %d symbol entries", this, symbolCount());
#endif
    Bytecode::writeInt32 (str, symbolCount());			// write Y2Namespace::m_symbols

    if (symbolCount() > 0)
    {
	for (unsigned int i = 0; i < symbolCount(); i++)
	{
	    SymbolEntryPtr entry = symbolEntry (i);
	    entry->toStream (str);				// write SymbolEntry
	}

	// if its a module, write the table

	if (isModule())	
	{
	    yTElist_t *tptr = m_tenvironment;
	    int tcount = 0;					// count the table entries
	    while (tptr)
	    {
		tcount++;
		tptr = tptr->next;
	    }
#if DO_DEBUG
	    y2debug ("Module with %d table entries", tcount);
#endif

	    Bytecode::writeInt32 (str, tcount);

	    tptr = m_tenvironment;
	    while (tptr)
	    {
		tptr->tentry->toStream (str);			// write the table entries
		tptr = tptr->next;
	    }
	}
    }

    m_point->toStream (str);

    u_int32_t count = statementCount();				// count statements
#if DO_DEBUG
    y2debug ("YBlock %p: %d statements", this, count);
#endif
    Bytecode::writeInt32 (str, count);

    stmtlist_t *stmt = m_statements;
    while (stmt)						// write statements
    {
#if DO_DEBUG
	y2debug ("toStream: %s", stmt->stmt->toString().c_str());
#endif
	stmt->stmt->toStream (str);				// YSImport will push it's namespace

	stmt = stmt->next;
    }

    Bytecode::popUptoNamespace (nameSpace());

#if DO_DEBUG
    y2debug ("YBlock::toStream done");
#endif
    return str;
}


bool
YBlock::isIncluded (string includefile) const
{
    return find (m_includes.begin (), m_includes.end (), includefile) != m_includes.end ();
}


void
YBlock::addIncluded (string includefile)
{
    if (find (m_includes.begin (), m_includes.end (), includefile) == m_includes.end ())
    {
	m_includes.push_back (includefile);
    }
}


Y2Function *
YBlock::createFunctionCall (const string name)
{
    TableEntry *func_te = table()->find (name.c_str (), SymbolEntry::c_function);
    
    // can't find the function definition
    if (func_te == NULL || !func_te->sentry ()->isFunction ()) 
	return NULL;
    
#if DO_DEBUG
    y2debug ("allocating new YEFunction %s", name.c_str ());
#endif
    return new YEFunction (func_te->sentry ());
}

// EOF
