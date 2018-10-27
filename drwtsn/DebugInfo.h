//***************************************************************
// DebugInfo.h
// 
// Debug Information Dumper
// Developed for CTR Systems
// By CISCORP, Inc. - Pittsburgh, PA
// Copyright 1997, by CISCORP, Inc.
//***************************************************************

void DebugOpenFile();
void DebugCloseFile();
void DebugWriteMessage( char *szMessage, int iDebugGrp );
void TransDumpOpenFile();
void TransDumpCloseFile();
void TransDumpWriteMessage( char *szMessage );
