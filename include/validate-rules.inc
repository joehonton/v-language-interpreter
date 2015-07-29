<?php
//=============================================================================
//
// File:         /var/vermont/include/evaluate-rules.inc
//
// Language:     PHP
//
// Contents:     Validate function
//
// Author:       Joe Honton © 2010
//
// Initial date: December 20, 2010
//
//=============================================================================


//-----------------------------------------------
//^ The ValidateRuleObjects scans all non-vmtResource based objects, in dependency
//  order, and evaluates them against their keyword rules
//
function ValidateRuleObjects()
{
	global $substitution_stage;
	global $config;
	$depth = 1;
	$s_tree = '';

	// loop over the object collection
	foreach( $substitution_stage as $so_name => $o )
		EvaluateOne( $o, $depth, $s_tree, 'rule', $substitution_stage );

	VerboseDependencyTree( $config, $s_tree );
}


//-----------------------------------------------
//^ The EvaluateOne function checks for object dependencies and recurses until
//  all dependencies are satisfied, then instantiates a vmtModule-derived
//  class based on the object's specified 'type', and finally kicks off the
//  keyword-rule evaluation by calling the vmtModule's execute() function.
//  This function can be called for two distinct stages: resource evaluation or rule evaluation
//
//> $o is a vmtManagedObj
//> $depth is the recursive level
//> $s_tree is the textual representation of the dependency tree for verbose output
//> $resource_or_rule is 'resource' to evaluate vmtResource objects or 'rule' to evaluate non-vmtResource objects
//> $object_collection is either the resource_stage collection (for 'resource') or the substitution_stage collection (for 'rule')
function EvaluateOne( $o, $depth, &$s_tree, $resource_or_rule, &$object_collection )
{
	global $commandline;
	global $config;
	global $internal_keywords;
	global $plugin_module_classes;
	
	// never execute a namespace, like [abcd:abcd]
	if ( $o->isNamespace() )
		return;

	if ( $resource_or_rule == 'resource' && !$o->isResource() )
		return;
		
	if ( $resource_or_rule == 'rule' && $o->isResource() )
		return;
		
	// build the textual representation of the dependency tree
	$indent = '';
	for ( $i = 1; $i < $depth; $i++ )
		$indent .= sprintf( "   " );
	$s_tree .= sprintf( "%s%s> %s:%s\n", $indent, $depth, $o->namespace_name, $o->object_name );
	
	// check to see if this object is already in-progress,
	// so the cyclic dependencies don't process infinitely
	if ( $o->state == 'in-progress' || $o->state == 'complete' )
		return;
	$o->state = 'in-progress';

	// process all dependencies
	$dependencies = explode( '|', array_key_exists('dependency', $o->kv) ? $o->kv['dependency'] : '' );
	foreach( $dependencies as $d )
	{
		$d = trim($d);
		if ( $d == '' || $d == 'undeclared' )
			continue;
		
		// dependencies can be specified with or without hard brackets, like "abcd:efgh" or "[abcd:efgh]"
		$last_char = strlen($d) - 1;
		if ( $d{0} == '[' && $d{$last_char} == ']' )
			$d = substr( $d, 1, $last_char-1 );
			
		// note that dependencies must be fully qualified namespace:object names, "efgh" is not enough
		if ( !array_key_exists( $d, $object_collection ) )
		{
			// issue error to stderr
			$line_number = array_key_exists('dependency', $o->tracking) ? $o->tracking['dependency'] : '';
			$comment = array_key_exists('comment', $o->kv) ? "({$o->kv['comment']})" : '';;
			$s = sprintf( "%s%s [%s:%s]->dependency there is no such namespace object [%s]", $line_number, $comment, $o->namespace_name, $o->object_name, $d);
			stderr( $s );

			// add an entry to the the dependency tree
			$indent = '';
			for ( $i = 1; $i < $depth+1; $i++ )
				$indent .= sprintf( "   " );
			$s_tree .= sprintf( "%s%s> %s (not found)\n", $indent, $depth+1, $d );
						
		}
		else
			EvaluateOne( $object_collection[$d], $depth+1, $s_tree, $resource_or_rule, $object_collection );
	}
	
	// Determine this object's type and make sure that the module exists
	if ( array_key_exists( 'type', $o->kv ) )
		$type = $o->kv['type'];
	else
	{
		// exact line number not possible, just use the line number of the first attribute
		$line_number = '';
		if ( count($o->tracking) > 0 )
			$line_number = reset($o->tracking);
		$comment = array_key_exists('comment', $o->kv) ? " ({$o->kv['comment']})" : '';
		stderr( sprintf( "%s%s [%s:%s]->type specification error missing 'type' keyword", $line_number, $comment, $o->namespace_name, $o->object_name ) );
		return;
	}
	
	if ( !array_key_exists( $type, $plugin_module_classes ) )
	{
		$line_number = array_key_exists('type', $o->tracking) ? $o->tracking['type'] : '';
		$comment = array_key_exists('comment', $o->kv) ? " ({$o->kv['comment']})" : '';
		stderr( sprintf( "%s%s [%s:%s]->type specification error unknown type '%s'", $line_number, $comment, $o->namespace_name, $o->object_name, $type ) );
		return;
	}

	//  the command line flag -x  and --noexec means 'Parse namespaces, but do not execute'
	if ( !in_array('x', $commandline['flags']) && !array_key_exists('noexec', $commandline['options']) )
	{
		// convert the 'type' attribute into a PHP class name
		$class_name = $plugin_module_classes[$type];				// virtualhost --> vmtVirtualhost
		$f = new $class_name( $o );									// this uses the __autoload function to find and include class vmtVirtualhost
		$f->execute();
	}
	
	// finish up
	$o->state = 'complete';
}

?>