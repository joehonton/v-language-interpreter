<?php
#==============================================================================
#
# File:         /var/vermont/modules/logfile-modules/base.inc
#
# Language:     Vermont plugin resource class
#
# Contents:     A base class for all resources modules
#
# Author:       Joe Honton © 2011
#
# Initial date: February 4, 2011
#
#==============================================================================

class vmtParseValues {};					// a fully dynamic class for the column values returned by parse

class vmtLogfileBase
{
	public $column_names = array();		// an array of column names that are parsed by the parse function
	
	//-------------------------------------------
	public function __construct()
	{
	}

	//-------------------------------------------
	//^ The parse function accepts a single line from a logfile and returns
	//  an array containing columns values.
	//> $line is a string of text in a format specific to this class's logfile
	//< $column_values is an array, that is returned by reference, containing
	//  the parsed values, or an empty string if the column value is missing.
	//<	Normally returns true, even when some columns could not be parsed.
	//  Returns false only on an unrecoverable error
	public function parse( $line, vmtParseValues &$column_values )
	{
		// each derived class should define this method
		return true;
	}
	
	//-------------------------------------------
	//^ The pop_fixed_length function gets a fixed length value out of the line
	//> $line is the full line to work with
	//> $offset, at call time, is a zero-based index into the line where the next value starts
	//< $offset, upon return, is the incremented index
	//> $num_chars is the number of characters to get
	//> $line_length is the actual length of the full line
	//< returns the value popped from the line, or an empty string on failure
	//
	public function pop_fixed_length( $line, &$offset, $num_chars, $line_length )
	{
		// don't overrun the length of the line
		if ( $offset >= $line_length )
			return '';
			
		$s = substr( $line, $offset, $num_chars );
		if ( $s == false )
			return '';
				
		$offset += $num_chars;
		return $s;
	}

	//-------------------------------------------
	//^ The pop_to_delimiter function gets a variable length value out of the line
	//> $line is the full line to work with
	//> $offset, at call time, is a zero-based index into the line where the next value starts
	//< $offset, upon return, is the incremented index
	//> $delimiter is the character that marks the end of the value.  The character itself is not returned and the offset points to it.
	//> $line_length is the actual length of the full line
	//< returns the value popped from the line, or an empty string on failure
	//
	public function pop_to_delimiter( $line, &$offset, $delimiter, $line_length )
	{
		// don't overrun the length of the line
		if ( $offset >= $line_length )
			return '';

		$pos = strpos( $line, $delimiter, $offset );
		if ( $pos == false )
		{
			$offset = $line_length;				// let the caller know that we've gobbled up the rest of the line
			$s = substr( $line, $offset );		// get the rest of the line
			return ( $s == false ) ? '' : $s;
		}
		$num_chars = $pos - $offset;
		$s = substr( $line, $offset, $num_chars );
		if ( $s == false )
			return '';
				
		$offset += $num_chars;
		return $s;
	}
	
	//-------------------------------------------
	//^ The extract_bracketed_substring function scans the given text, and extracts
	//  the sub-text that is contained within the opening and closing brackets.
	//  The function returns three strings: the text before, the bracketed sub-text
	//  and the text after.  The brackets themselves are discarded.
	//> $text is the input string to examine
	//> $open_bracket is something like '[', '{', '('
	//> $close_bracket is something like ']', '}', ')'
	//< returns a 3-tuple with three strings
	//
	public function extract_bracketed_substring( $text, $open_bracket, $close_bracket )
	{
		$tuple = array( '', '', $text );	// the default, if something fails
		
		$pos_open = strpos( $text, $open_bracket );
		if ( $pos_open === false )
			return $tuple;
		
		$pos_close = strpos( $text, $close_bracket, $pos_open+1 );
		if ( $pos_close === false )
			return $tuple;
	
		$tuple[0] = substr( $text, 0, $pos_open );
		$tuple[1] = substr( $text, $pos_open+1, ($pos_close - $pos_open - 1) );
		$tuple[2] = substr( $text, $pos_close+1 );
		return $tuple;
	}
	
	
	//-------------------------------------------
	//^ The pop_keyword_to_keyword function pulls a value out of a string where
	//  the value is preceded by a keyword and continues up to a delimiter.
	//  For example, the line containing "PROTO=UDP SPT=24895 DPT=53", when given a
	//  keyword of 'SPT=' and a delimiter of 'DPT=' will return 24895.
	//> $line is the string to parse
	//> $offset, upon call, is the offset into the line where parsing should begin
	//< $offset, upon exit, is the first character of the delimiter
	//> $keyword is the string to look for in the line, which signals the start of the value,
	//   use an empty string to start at the specified offset from the beginning of the line.
	//> $delimiter is the string to look for which signals the end of the value,
	//   if not found, the return string includes everything to the end of the line
	//   to explicitly specify end of line, use an empty string ''
	//> $line_length is the length of the line, used to prevent scanning past the end of the line
	//< returns the value found, or an empty string if not found, or echos the keyword itself if the keyword was found but the value is blank
	public function pop_keyword_to_keyword( $line, &$offset, $keyword, $delimiter, $line_length )
	{
		// don't overrun the length of the line
		if ( $offset >= $line_length )
			return '';

		if ( $keyword == '' )
			$pos_keyword = $offset;			// no keyword specified, so start from the current position
		else
			$pos_keyword = strpos( $line, $keyword, $offset );
		
		$s = '';
		if ( $pos_keyword !== false )
		{
			$pos_keyword_len = strlen($keyword);
			$pos_value = $pos_keyword + $pos_keyword_len;				// the starting position of the value we're looking for
			
			if ( $delimiter == '' )
				$pos_delimiter = false;
			else
				$pos_delimiter = strpos( $line, $delimiter, $pos_value );
			
			if ( $pos_delimiter !== false )
			{
				$value_len = $pos_delimiter - $pos_value;
				$s = trim( substr( $line, $pos_value, $value_len ) );
				$offset = $pos_value + $value_len;
			}
			else
			{
				$s = trim( substr( $line, $pos_value ) );	// to the end of the string
				$offset = strlen($line);
			}
		}
		
		if ( $s == '' )
			return $keyword;		// the special rule for situations where the presence or absence of the keyword itself is the signal, like DF or SYN
		else
			return $s;				// the normal case, return the extracted value
	}
	
}
?>