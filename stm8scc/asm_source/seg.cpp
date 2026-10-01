#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
extern "C" {
#include "define.h"
#include "common.h"
#include "asm.h"
}

#include "symbol.h"
#include "seg.h"

Cseg *segList = NULL;
Cseg *curSeg;
static int seqNum = 0;

/////////////////////////////////////////////////////////////////////
Cseg :: Cseg ()		// class constructor
{
	lines = NULL;
	next  = NULL;

	if ( segList == NULL )
		segList = this;
	else
		curSeg->next = this;

	curSeg = this;
	seq_num = seqNum++;
}

/////////////////////////////////////////////////////////////////////
Cseg :: ~Cseg ()	// class destructor
{
	freeLines (lines);
}

/////////////////////////////////////////////////////////////////////
void Cseg :: addLine (line_t *lptr)
{
	line_t *p = lines;

	if ( lines == NULL )
		lines = lptr;
	else
	{
		while ( p->next != NULL )
			p = p->next;

		p->next = lptr;
	}
}

/////////////////////////////////////////////////////////////////////
int Cseg :: type (void)
{
    if ( STR_MATCHn (name(), (char*)"CODE", 4) )
		return CODE_SEGMENT;

    if ( STR_MATCHn (name(), (char*)"BANK", 4) )
		return DATA_SEGMENT;

    if ( STR_MATCHn (name(), (char*)"EPROM", 5) )
		return EPROM_SEGMENT;

    if ( STR_MATCHn (name(), (char*)"FUSE", 4) )
		return FUSE_SEGMENT;

    if ( STR_MATCHn (name(), (char*)"CONST", 5) )
		return CONST_SEGMENT;

	return 0;
}

/////////////////////////////////////////////////////////////////////
char *Cseg :: name (void)
{
	if ( lines == NULL || lines->inst != SEGMENT )
		return (char*)"";

	return lines->area->name;
}

/////////////////////////////////////////////////////////////////////
bool Cseg :: isABS (void)
{
	switch ( type () )
	{
		case CODE_SEGMENT:
		case DATA_SEGMENT:
		case EPROM_SEGMENT:
		case FUSE_SEGMENT:
		case CONST_SEGMENT:
			return lines->area->isABS? true: false;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////
bool Cseg :: isREL (void)
{
	switch ( type () )
	{
		case CODE_SEGMENT:
		case DATA_SEGMENT:
		case EPROM_SEGMENT:
		case FUSE_SEGMENT:
		case CONST_SEGMENT:
			if ( !lines->area->isABS && !lines->area->isOVR )
				return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////
bool Cseg :: isOVR (void)
{
	switch ( type () )
	{
		case CODE_SEGMENT:
		case DATA_SEGMENT:
		case EPROM_SEGMENT:
		case FUSE_SEGMENT:
		case CONST_SEGMENT:
			return lines->area->isOVR? true: false;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////
bool Cseg :: isBEG (void)
{
	switch ( type () )
	{
		case CODE_SEGMENT:
		case DATA_SEGMENT:
		case EPROM_SEGMENT:
		case FUSE_SEGMENT:
		case CONST_SEGMENT:
			return lines->area->isBEG? true: false;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////
bool Cseg :: isEND (void)
{
	switch ( type () )
	{
		case CODE_SEGMENT:
		case DATA_SEGMENT:
		case EPROM_SEGMENT:
		case FUSE_SEGMENT:
		case CONST_SEGMENT:
			return lines->area->isEND? true: false;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////
void clearupSegments (void)
{
	while ( segList != NULL )
	{
		Cseg *sp = segList->next;

		delete segList;
		segList = sp;
	}
	seqNum = 0;
}
