<?php
session_start();
$method=getenv('REQUEST_METHOD');
//$fp = fopen("/var/log/ldap-login.log", "a");
//$date = date("d/M/Y H:i:s");
//fwrite($fp, "LOGN: $REMOTE_ADDR $login $date $HTTP_USER_AGENT $HTTP_REFERER $REQUEST_METHOD \n");
//fclose($fp);

function authenticate($user, $pw) {
	include ("config.inc.php");
	global $handle;

	$query="SELECT * FROM adminuser WHERE username='$user' AND password='$pw'";
	//$query="SELECT * FROM adminuser";
	$handle=mysql_connect($MYSQL_HOST,$MYSQL_USER,$MYSQL_PASSWD);
	$result=mysql_db_query($MYSQL_DB,$query, $handle);
	$cnt=mysql_num_rows($result);
	$username=mysql_result($result,0,'username');
	$password=mysql_result($result,0,'password');

	if ($username==$user and $password==$pw){
		$auth=true;
	}
if ($auth){
//	if ($user=="blubber"){
//		print $user;
		return TRUE;
	}
	else {
		return FALSE;
	}

}

$session_ok = FALSE;

if (isset($login) and isset($password)) {

  // Log access
	$fp = fopen("/var/log/ldap-login.log", "a");
	$date = date("d/M/Y H:i:s");
	fwrite($fp, "LOGN: $REMOTE_ADDR $login $date $HTTP_USER_AGENT $HTTP_REFERER $REQUEST_METHOD \n");
	fclose($fp);

//	print "login".$login;

	if (authenticate($login, $password)) {
//	if ($login=="luc"){

        	// Log successfull login
        	$fp = fopen("/var/log/ldap-login.log", "a");
         	$date = date("d/M/Y H:i:s");
         	fwrite($fp, "PASS: $REMOTE_ADDR $login $date $HTTP_USER_AGENT $HTTP_REFERER $REQUEST_METHOD \n");
         	fclose($fp);

         	$session_ok = TRUE;
		$user = $login;

         	session_register("session_ok");
		session_register("user");
		$SID=session_id();
		$query="UPDATE adminuser SET SID='$SID' WHERE username='$user'";
		$result=mysql_db_query($MYSQL_DB,$query, $handle);
		

	 	header ("Location: index.php");

//		print "Authentication sucessful";
		break;
        } 

	else {

        // Log login failure
        $fp = fopen("/var/log/ldap-login.log", "a");
        $date = date("d/M/Y H:i:s");
        fwrite($fp, "FAIL: $REMOTE_ADDR $login $date $HTTP_USER_AGENT $HTTP_REFERER $REQUEST_METHOD \n");
        fclose($fp);
	print "Auth failed";	
	unset($session_ok);
	session_unregister("session_ok");
	header ("Location: logout.php");	
	}
} 
else {

	print "<center><h4><font face=Verdana,Geneva,Arial,Helvetica,sans-serif>Web-cyradm is for authorized users only. <br>Make sure you entered the right password. <br>Push the back button in your browser to try again. Your attempt to login has been stored.</font></h4></center>";
}


?>

<!-- ###################################### End auth.inc.php ################################################
