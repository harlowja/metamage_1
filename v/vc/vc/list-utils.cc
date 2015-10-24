/*
	list-utils.cc
	-------------
*/

#include "vc/list-utils.hh"


namespace vc
{
	
	static const Value empty_list = Value_empty_list;
	
	
	const Value& first( const Value& list )
	{
		if ( list.type == Value_pair )
		{
			Expr* expr = list.expr.get();
			
			if ( expr->op == Op_list )
			{
				return expr->left;
			}
		}
		
		return list;  // not a (non-empty) list
	}
	
	const Value& rest( const Value& list )
	{
		if ( list.type == Value_pair )
		{
			Expr* expr = list.expr.get();
			
			if ( expr->op == Op_list )
			{
				return expr->right;
			}
		}
		
		return empty_list;
	}
	
	unsigned long count( const Value& list )
	{
		if ( list.type != Value_pair )
		{
			return ! is_empty( list );
		}
		
		Expr* expr = list.expr.get();
		
		if ( expr->op != Op_list )
		{
			return 1;
		}
		
		unsigned long total = count( expr->left );
		
		while ( Expr* next = expr->right.expr.get() )
		{
			if ( next->op != Op_list )
			{
				return total + 1;
			}
			
			total += count( next->left );
			
			expr = next;
		}
		
		return total + count( expr->right );
	}
	
	Value make_pair( const Value& left, const Value& right )
	{
		if ( left.type == Value_pair )
		{
			Expr& expr = *left.expr.get();
			
			if ( expr.op == Op_list )
			{
				return Value( expr.left, make_pair( expr.right, right ) );
			}
		}
		
		return Value( left, right );
	}
	
}
