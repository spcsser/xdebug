--TEST--
Test to check assignments are recorded correctly in od mode
--INI--
xdebug.auto_trace=0
xdebug.trace_format=11
--FILE--
<?php
$string='/tmp/' . uniqid('xdt',TRUE);
$tf = xdebug_start_trace($string);

function test($a, $b, $c)
{
	$d = 89;
	$d++;
	$d=--$d;
	$a += $b;
	$c /= 3;
	$b *= 9;
	$a =& $c;
	$text='Test';
	$text.=' it!';
}

test(1, 2, 3);

xdebug_stop_trace();
echo file_get_contents($tf);
$string.='_data.xt';
echo file_get_contents($string);
unlink($tf);
unlink($string);
--EXPECTF--
Version: %d.%d.%d-%s
File format: 12
TRACE START [%d-%d-%d %d:%d:%d]

{"lvl":2,"aid":2,"tme":%d.%d,"mem":%d,"atp":1}
{"lvl":1,"aid":3,"atp":2,"tme":%d.%d,"mem":%d,"nme":"$tf","fle":"%sodb_assignment.php","lne":3}
{"lvl":2,"aid":4,"tme":%d.%d,"mem":%d,"atp":0,"nme":"test","ext":1,"ftp":1,"fle":"%sodb_assignment.php","lne":18}
{"lvl":2,"aid":5,"atp":2,"tme":%d.%d,"mem":%d,"nme":"$d","fle":"%sodb_assignment.php","lne":7}
{"lvl":2,"aid":6,"atp":2,"tme":%d.%d,"mem":%d,"nme":"$d","fle":"%sodb_assignment.php","lne":8}
{"lvl":2,"aid":7,"atp":2,"tme":%d.%d,"mem":%d,"nme":"$d","fle":"%sodb_assignment.php","lne":9}
{"lvl":2,"aid":8,"atp":2,"tme":%d.%d,"mem":%d,"nme":"$d","fle":"%sodb_assignment.php","lne":9}
{"lvl":2,"aid":9,"atp":2,"tme":%d.%d,"mem":%d,"nme":"$a","fle":"%sodb_assignment.php","lne":10}
{"lvl":2,"aid":10,"atp":2,"tme":%d.%d,"mem":%d,"nme":"$c","fle":"%sodb_assignment.php","lne":11}
{"lvl":2,"aid":11,"atp":2,"tme":%d.%d,"mem":%d,"nme":"$b","fle":"%sodb_assignment.php","lne":12}
{"lvl":2,"aid":12,"atp":2,"tme":%d.%d,"mem":%d,"nme":"$a","fle":"%sodb_assignment.php","lne":13}
{"lvl":2,"aid":13,"atp":2,"tme":%d.%d,"mem":%d,"nme":"$text","fle":"%sodb_assignment.php","lne":14}
{"lvl":2,"aid":14,"atp":2,"tme":%d.%d,"mem":%d,"nme":"$text","fle":"%sodb_assignment.php","lne":15}
{"lvl":2,"aid":4,"tme":%d.%d,"mem":%d,"atp":1}
{"lvl":2,"aid":15,"tme":%d.%d,"mem":%d,"atp":0,"nme":"xdebug_stop_trace","ext":0,"ftp":1,"fle":"%sodb_assignment.php","lne":20}
{"aid":3,"atp":2,"id":0,"nme":"$tf","val":{"typ":"string","id":%d,"val":"'/tmp/xdt%s.%d.xt'"}}
{"aid":4,"atp":0,"obj":{"typ":"NULL","id":0},"arg":[{"nme":"$a","val":{"typ":"int","id":%d,"val":1}},{"nme":"$b","val":{"typ":"int","id":%d,"val":2}},{"nme":"$c","val":{"typ":"int","id":%d,"val":3}}]}
{"aid":5,"atp":2,"id":0,"nme":"$d","val":{"typ":"int","id":%d,"val":89}}
{"aid":6,"atp":2,"id":0,"nme":"$d","val":{"typ":"int","id":%d,"val":90}}
{"aid":7,"atp":2,"id":0,"nme":"$d","val":{"typ":"int","id":%d,"val":89}}
{"aid":8,"atp":2,"id":0,"nme":"$d","val":{"typ":"int","id":%d,"val":89}}
{"aid":9,"atp":2,"id":0,"nme":"$a","val":{"typ":"int","id":%d,"val":2}}
{"aid":10,"atp":2,"id":0,"nme":"$c","val":{"typ":"int","id":%d,"val":3}}
{"aid":11,"atp":2,"id":0,"nme":"$b","val":{"typ":"int","id":%d,"val":9}}
{"aid":12,"atp":2,"id":0,"nme":"$a","val":{"typ":"int","id":%d,"val":1}}
{"aid":13,"atp":2,"id":0,"nme":"$text","val":{"typ":"string","id":%d%d,"val":"'Test'"}}
{"aid":14,"atp":2,"id":0,"nme":"$text","val":{"typ":"string","id":%d,"val":"'Test it!'"}}
{"aid":4,"atp":1,"val":{"typ":"NULL","id":%d}}
{"aid":15,"atp":0,"obj":{"typ":"NULL","id":0},"arg":[]}
