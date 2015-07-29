<?php
//=============================================================================
//
// File:         /var/vermont/include/substitution.inc
//
// Language:     PHP
//
// Contents:     Function to scan all object keywords looking for substitution
//               variables and replacing those variables with keyword values.
//               Substitution variables can be specified in three forms:
//                 [keyword] this will look for a keyword within the current object
//                 [object.keyword] this will look for a keyword within the
//                   specified object
//                 [namespace:object.keyword] this will look for a keyword
//                   within the specified object in another namespace
//
// Author:       Joe Honton © 2011
//
// Initial date: January 31, 2011
//
//=============================================================================


//-----------------------------------------------
//^ The ApplyStaticSubstitutions function. Loop through the entire resource-stage collection
//  to build a new collection of "substitutes" with all substitution variables resolved.
function ApplyStaticSubstitutions()
{
	global $config;
	global $resource_stage;
	global $substitution_stage;
			
	foreach ( $resource_stage as $ro )
	{
		$so_name = sprintf( "%s:%s", $ro->namespace_name, $ro->object_name );
		$so = new vmtManagedObject( $ro->namespace_name, $ro->object_name );
		$substitution_stage[$so_name] = $so;

		// copy each keyword-value pair from the resource-stage object to the new substitution-stage object
		foreach ( $ro->kv as $keyword => $value )
		{
			$recursion_count = 0;
			// Look for embedded substitution variables, a single instance at a time
			// this regex uses two alternate patterns \$.+?\s and \$.+?$, to look for
			// dollar-sign prefixed strings that end in whitespace or end at the end of the line
			// The question mark in each is to make the sequence non-greedy.
			// Careful, don't use double quotes to surround the literal because the dollar-sign won't survive PHP's literal parser.
			while ( preg_match( '/\$.+?\s|\$.+?$/', $value, $matches ) == 1 )
			{
				if ( $recursion_count > 2 )
					break;
				$substitution_variable = trim($matches[0]);
				$substitution_value = LookupSubstitutionValue( $ro->namespace_name, $ro->object_name, $substitution_variable );
				
				// if the substitution value itself contains embedded variables, the next iteration of the foreach will
				// evaluate it, but to prevent an infinite loop, limit recursion to a small number.
				if ( strpos( $substitution_value, '$' ) !== false )
					$recursion_count++;
				$value = str_replace( $substitution_variable, $substitution_value, $value );
			}
			
			// now go back and restore any variables that were not found by
			// LookupSubstitutionValue() so that embedded dollar signs are
			// allowed even if they don't refer to an object keyword.
			while ( preg_match( '/\x1B.+?\s|\x1B.+?$/', $value, $matches ) == 1 )
			{
				$restored_value = str_replace( "\x1B", '$', $matches[0] );
				$value = str_replace( $matches[0], $restored_value, $value );
			}
			
			// save the modified value to the new collection's object
			$so->kv[$keyword] = $value;
		}
		
		// copy the line-number tracking array
		foreach ( $ro->tracking as $keyword => $value )
			$so->tracking[$keyword] = $value;
	}

	VerboseSubstitutionObjects( $config, $substitution_stage );
}



//-----------------------------------------------
//^ The LookupSubstitutionValue function parses the given dollar-sign prefixed
//  substitution variable into three parts and uses that information
//  to obtain a value from the collection of resource_stage objects.
//  The three values are syntactically composed of namespace,
//  object, and keyword.
//> $default_namespace is the namespace to use if the substitution variable does not specify one
//> $default_object is the object to use if the substitution variable does not specify one
//> $substitution_variable is something like [namespace:object.keyword]
//< returns the value as obtained from the resource_stage collection
//< returns 'undeclared' if not found
//
function LookupSubstitutionValue( $default_namespace, $default_object, $substitution_variable )
{
	global $resource_stage;
	
	$namespace_name = $default_namespace;
	$object_name = $default_object;
	
	// remove the dollar-sign prefix
	if ( $substitution_variable{0} == '$' )
		$substitution_variable = substr( $substitution_variable, 1 );

	$pos_colon = strpos( $substitution_variable, ':' );
	$pos_period = strpos( $substitution_variable, '.' );
	if ( $pos_colon !== false && $pos_period === false )
	{
		echo "'$substitution_variable' specifies a namespace and a keyword without an specifying an object, use a period instead of a colon\n";
	}
	
	// pull out the namespace name, if it was specified
	if ( $pos_colon !== false )
	{
		$namespace_name = substr( $substitution_variable, 0, $pos_colon );
		$substitution_variable = substr( $substitution_variable, $pos_colon+1 );
	}

	// pull out the object name, if it was specified
	$pos_period = strpos( $substitution_variable, '.' );
	if ( $pos_period !== false )
	{
		$object_name = substr( $substitution_variable, 0, $pos_period );
		$substitution_variable = substr( $substitution_variable, $pos_period+1 );
	}

	$keyword_name = $substitution_variable;

	// look for the namespace-object in the resource_stage collection
	$ro_name = sprintf( "%s:%s", $namespace_name, $object_name );
	if ( array_key_exists( $ro_name, $resource_stage ) )
	{
		$o = $resource_stage[$ro_name];
		
		// now look for the keyword within the resource-stage object
		if ( array_key_exists( $keyword_name, $o->kv ) )
			return $o->kv[$keyword_name];
		else
			return "\x1B" . $substitution_variable; 	// keep the missing variable in-situ, but used a temporary character to stand in for the dollar sign
	}
	else
		return "\x1B" . $substitution_variable; 		// keep the missing variable in-situ, but used a temporary character to stand in for the dollar sign
}

?>