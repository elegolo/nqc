/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * The Initial Developer of this code is David Baum.
 * Portions created by David Baum are Copyright (C) 1999 David Baum.
 * All Rights Reserved.
 *
 * Portions created by John Hansen are Copyright (C) 2005 John Hansen.
 * All Rights Reserved.
 *
 */
#ifndef __Expr_h
#define __Expr_h

#ifndef __RCX_Constants_h
#include "RCX_Constants.h"
#endif

#ifndef __AutoFree_h
#include "AutoFree.h"
#endif

#ifndef __Variable_h
#include "Variable.h"
#endif

#ifndef __LexLocation_h
#include "LexLocation.h"
#endif

#include <vector>

using std::vector;

class Bytecode;
class Mapping;
class RCX_Target;

/**
 * The Expr class is the base class for expressions.  It declares
 * several important virtual methods that are used by the rest of
 * the compiler to determine properties and generate code for
 * general expressions.
 */
class Expr : public AutoFree
{
public:
	enum {
		kIllegalEA = -1
	};

			Expr(const LexLocation &loc) : fLoc(loc) {}
	virtual	~Expr() = 0;

	const LexLocation&	GetLoc() const { return fLoc; }
	void				SetLoc(const LexLocation &loc)	{ fLoc = loc; }

    /*
     * Used to clone an expression while making
     * the substitutions defined by a mapping.  This is primarily
     * used during expansion of inline functions (see CallStmt).
     */
	virtual Expr*		Clone(Mapping *) const = 0;

	virtual bool		Evaluate(int & /*value */) const	{ return false; }

    /*
     * Determine if an expression is
     * a potential candidate as an LValue.  This does not guarantee
     * that the expression is a legal LValue for a given target, but
     * merely that it obeys the semantic rules for being an LValue.
     * This includes making sure that variables are non-const and
     * that the expression is not a calculated value.  Most expressions
     * are not potential LValues, so the default implementation is to
     * return false.
     */
	virtual bool		PotentialLValue() const	{ return false; }
	virtual int			GetLValue() const	{ return kIllegalVar; }
			RCX_Value	GetStaticEA() const;

	virtual bool		PromiseConstant() const { return true; }
	virtual bool		Contains(int /* var */) const { return false; }
        virtual bool            LValueIsPointer() const { return false; }

	/// Append sub-expressions into vector
	virtual void		GetExprs(vector<Expr*> & /* v */) const { }

	// calls to emit code
	RCX_Value	EmitAny(Bytecode &b) const;
	bool		EmitTo(Bytecode &b, int dst) const;
	RCX_Value	EmitConstrained(Bytecode &b, long mask, bool canUseLocals=true) const;
	bool		EmitSide(Bytecode &b) const;
	bool		EmitBranch(Bytecode &b, int label, bool condition) const;
	RCX_Value	EmitMath(Bytecode &b) const;

protected:
	// virtual code emit methods for derrived classes
	virtual RCX_Value	EmitAny_(Bytecode &b) const = 0;
	virtual bool		EmitTo_(Bytecode &b, int dst) const;
	virtual bool		EmitSide_(Bytecode &) const;
	virtual bool		EmitBranch_(Bytecode &b, int label, bool condition) const;

	virtual RCX_Value	GetStaticEA_() const;


	/// Helper routines for EmitAny_() implementation
	RCX_Value	EmitBoolAny(Bytecode &b) const;

    /// Helper routines for EmitTo_() implementation
	bool		EmitBoolTo(Bytecode &b, int dst) const;


	/// Utility function
	int GetTempVar(Bytecode &b, bool canUseLocals=true) const;

private:
	LexLocation		fLoc;
};


/// use this to build a typemask for EmitConstrained()
#define TYPEMASK(t)	(1L << (t))

// masks for EmitConstrained()
#define TEST_MASK	(~(TYPEMASK(kRCX_RandomType) + \
						TYPEMASK(kRCX_ProgramType) + \
						TYPEMASK(kRCX_AGCType)))
#define SET_MASK	-1


template <class OP> void Apply(Expr *base, OP &op)
{
	if (op(base)) {
		vector<Expr*> v;
		base->GetExprs(v);

		int n = v.size();
		for (int i=0; i<n; ++i)
			Apply(v[i], op);
	}
}


#endif
