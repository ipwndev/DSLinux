///////////////////////////////////////////////////
var ZIPOBJ=null;

function CreateZIPOBJ()
{
if (ZIPOBJ)return true;	
ZIPOBJ=new ActiveXObject("SAWZip.Archive");
if (!ZIPOBJ)
	{
	alert ("can't find UnZip component");
	return false;
	}
return true;		
}

function GetListOfZippedFiles(ArchName)
{
if (!ZIPOBJ) if (!CreateZIPOBJ())return null;
//if (!ZIPOBJ) {alert('error! ZIPOBJ=NULL');return null;}
if (ArchName=='') {alert('error! ArchName=""');return null;}
ZIPOBJ.Open(ArchName);

var o=ZIPOBJ.Files;	
var l=o.Count;
var a=new Array(l);
for (var i=0; i<l; i++)
	{
	a[i]=new Array();
	a[i]['Index']=o.Item(i).Index;
	a[i]['Name']=o.Item(i).Name;
	a[i]['UncompressedSize']=o.Item(i).UncompressedSize;
	a[i]['Directory']=o.Item(i).Directory;
	a[i]['Comment']=o.Item(i).Comment;
	a[i]['CompressedSize']=o.Item(i).CompressedSize;
	a[i]['Crc32']=o.Item(i).Crc32;
	a[i]['FullPath']=o.Item(i).FullPath;
	a[i]['Level']=o.Item(i).Level;
	}
ZIPOBJ.Close();	
return a;
}

function Extract1ZippedFile(aName,fIndex,fname,foldername,WithPath)
{
if (!ZIPOBJ) {alert('error! ZIPOBJ=NULL');return false;}
if (aName=='') {alert('error! aName=""');return false;}

var ind=fIndex;
if (ind==-1)//try to find index
	{
	ind=FindIndexOfZippedFile(aName,fname);
	if (ind==-1) {return false;}
	}
	
ZIPOBJ.Open(aName);

	var fullName=ZIPOBJ.Files.Item(ind).Name;
	var wfullName=DosToWin(fullName);
	var shortName=ExtractFileName(fullName);
	var wshortName=ExtractFileName(wfullName);
	
	ZIPOBJ.Files.Item(ind).FullPath=WithPath;
	try{
		ZIPOBJ.Files.Item(ind).Extract(foldername);
	}catch(e){}
	
	if (WithPath)
		{
		if( (foldername + fullName) != (foldername+wfullName))
		if(TRRenameFile( foldername + fullName , foldername+wfullName))
			{
			ZIPOBJ.Close();
			alert('rename error1\n'+foldername + fullName+'\nto\n'+foldername+wfullName);
			return false;	
			}
		}
	else
		{
		if (( foldername + shortName)!= (foldername+wshortName))
		if(TRRenameFile( foldername + shortName, foldername+wshortName))
			{
			ZIPOBJ.Close();
			alert('rename error2\n'+foldername + shortName+'\nto\n'+foldername+wshortName);
			return false;
			}
		}		
ZIPOBJ.Close();
return true;
}


function ExtractAllZippedFiles(aName,foldername,WithPath,excludeMask)
{
if (!ZIPOBJ) if (!CreateZIPOBJ())return false;
if (aName=='') {alert('error! aName=""');return false;}

var re;
if (excludeMask) re=new RegExp(excludeMask,'i');


var a=GetListOfZippedFiles(aName);
if (!a) return false;
var l=a.length;
for (var i=0; i<l ;i++)
	{
	if (excludeMask)
		{
		var s=a[i]['Name'];
		if (s.search(re)==-1) 
			Extract1ZippedFile(aName,i,'',foldername,WithPath);
		}
	else
		{		
		Extract1ZippedFile(aName,i,'',foldername,WithPath);
		}
	}
return true;
}


function FindIndexOfZippedFile(aName,fname){
var a=GetListOfZippedFiles(aName);
if (!a) return -1;
var l=a.length;
for (var i=0; i<l ;i++)
	if (a[i]['Name']==fname) return i;
return -1;	
}

function ZIPAddFileByName (aName,fNameMask,level,fullPath,refresh) 
{
if (!ZIPOBJ) if (!CreateZIPOBJ())return false;

if (''==HELPER.GetListOfFiles(aName)) ZIPOBJ.Create(aName);	

ZIPOBJ.Open(aName);
//alert(fNameMask+','+level+','+fullPath+','+refresh);
ZIPOBJ.Files.AddFileByName (fNameMask,Number(level),fullPath,refresh);
//alert(1);
ZIPOBJ.Close();
return true;
}
///works ???????
function ZIPExtractToString(aName,fIndex,fname)
{
if (!ZIPOBJ) if (!CreateZIPOBJ())return null;

if (aName=='') {alert('error! aName=""');return null;}

var ind=fIndex;
if (ind==-1)//try to find index
	{
	ind=FindIndexOfZippedFile(aName,fname);
	if (ind==-1) {return null;}
	}
	
ZIPOBJ.Open(aName);
var s=ZIPOBJ.Files.Item(ind).ExtractToString();
ZIPOBJ.Close();
return s;
}

function ZIPCreate(aName){
if (!ZIPOBJ) {alert('error! ZIPOBJ=NULL');return null;}
ZIPOBJ.Create(aName);
}

function ZIPRemoveFile(aName,fIndex,fname){
if (!ZIPOBJ) {alert('error! ZIPOBJ=NULL');return null;}
if (aName=='') {alert('error! aName=""');return null;}

var ind=fIndex;
if (ind==-1)//try to find index
	{
	ind=FindIndexOfZippedFile(aName,fname);
	if (ind==-1) {return null;}
	}
	
ZIPOBJ.Open(aName);
ZIPOBJ.Files.Remove(ind);//Item().
ZIPOBJ.Close();
return true;

}

