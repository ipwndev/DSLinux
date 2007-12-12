/* cmds.h
 *
 * Copyright (c) 1992-2006 by Mike Gleason.
 * All rights reserved.
 * 
 */

/* cmds.c */
int PromptForBookmarkName(BookmarkPtr);
void CurrentURL(char *, size_t, int);
void FillBookmarkInfo(BookmarkPtr);
void SaveCurrentAsBookmark(void);
void SaveUnsavedBookmark(void);
void BookmarkCmd(const int, char **const, const CommandPtr, const ArgvInfoPtr);
void CatCmd(const int, char **const, const CommandPtr, const ArgvInfoPtr);
void PrintResp(FTPLineListPtr);
int nFTPChdirAndGetCWD(const FTPCIPtr, const char *, const int);
int Chdirs(FTPCIPtr cip, const char *const cdCwd);
void BGStartCmd(const int, char **const, const CommandPtr, const ArgvInfoPtr);
void ChdirCmd(const int, char **const, const CommandPtr, const ArgvInfoPtr);
void ChmodCmd(const int, char **const, const CommandPtr, const ArgvInfoPtr);
void CloseCmd(const int, char **const, const CommandPtr, const ArgvInfoPtr);
void DebugCmd(const int, char **const, const CommandPtr, const ArgvInfoPtr);
void DeleteCmd(const int, char **const, const CommandPtr, const ArgvInfoPtr);
void EchoCmd(const int, char **const, const CommandPtr, const ArgvInfoPtr);
void EditCmd(const int, char **const, const CommandPtr, const ArgvInfoPtr);
void InitTransferType(void);
void GetCmd(const int, char **const, const CommandPtr, const ArgvInfoPtr);
void HelpCmd(const int, char **const, const CommandPtr, const ArgvInfoPtr);
void HostsCmd(const int, char **const, const CommandPtr, const ArgvInfoPtr);
void JobsCmd(const int, char **const, const CommandPtr, const ArgvInfoPtr);
void ListCmd(const int, char **const, const CommandPtr, const ArgvInfoPtr);
void LocalChdirCmd(const int, char **const, const CommandPtr, const ArgvInfoPtr);
void LocalListCmd(const int, char **const, const CommandPtr, const ArgvInfoPtr);
void LocalChmodCmd(const int, char **const, const CommandPtr, const ArgvInfoPtr);
void LocalMkdirCmd(const int, char **const, const CommandPtr, const ArgvInfoPtr);
void LocalPageCmd(const int, char **const, const CommandPtr, const ArgvInfoPtr);
void LocalRenameCmd(const int, char **const, const CommandPtr, const ArgvInfoPtr);
void LocalRmCmd(const int, char **const, const CommandPtr, const ArgvInfoPtr);
void LocalRmdirCmd(const int, char **const, const CommandPtr, const ArgvInfoPtr);
void LocalPwdCmd(const int, char **const, const CommandPtr, const ArgvInfoPtr);
void LookupCmd(const int, char **const, const CommandPtr, const ArgvInfoPtr);
void MkdirCmd(const int, char **const, const CommandPtr, const ArgvInfoPtr);
void MlsCmd(const int, char **const, const CommandPtr, const ArgvInfoPtr);
int DoOpen(void);
void OpenCmd(const int, char **const, const CommandPtr, const ArgvInfoPtr);
void PageCmd(const int, char **const, const CommandPtr, const ArgvInfoPtr);
void PassiveCmd(const int, char **const, const CommandPtr, const ArgvInfoPtr);
void PutCmd(const int, char **const, const CommandPtr, const ArgvInfoPtr);
void PwdCmd(const int, char **const, const CommandPtr, const ArgvInfoPtr);
void QuitCmd(const int, char **const, const CommandPtr, const ArgvInfoPtr);
void QuoteCmd(const int, char **const, const CommandPtr, const ArgvInfoPtr);
void RGlobCmd(const int, char **const, const CommandPtr, const ArgvInfoPtr);
void RenameCmd(const int, char **const, const CommandPtr, const ArgvInfoPtr);
void RmdirCmd(const int, char **const, const CommandPtr, const ArgvInfoPtr);
void RmtHelpCmd(const int, char **const, const CommandPtr, const ArgvInfoPtr);
void SetCmd(const int, char **const, const CommandPtr, const ArgvInfoPtr);
void ShellCmd(const int, char **const, const CommandPtr, const ArgvInfoPtr);
void SiteCmd(const int, char **const, const CommandPtr, const ArgvInfoPtr);
void SpoolGetCmd(const int, char **const, const CommandPtr, const ArgvInfoPtr);
void SpoolPutCmd(const int, char **const, const CommandPtr, const ArgvInfoPtr);
void SymlinkCmd(const int, char **const, const CommandPtr, const ArgvInfoPtr);
void TypeCmd(const int, char **const, const CommandPtr, const ArgvInfoPtr);
void UmaskCmd(const int, char **const, const CommandPtr, const ArgvInfoPtr);
void VersionCmd(const int, char **const, const CommandPtr, const ArgvInfoPtr);

int AskYesNoQuestion(const int, const char *const, ...)
#if (defined(__GNUC__)) && (__GNUC__ >= 2)
__attribute__ ((format (printf, 2, 3)))
#endif
;

/* vim: set noet sw=8: */
