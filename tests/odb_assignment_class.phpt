--TEST--
Test to check assignments are recorded correctly in od mode
--INI--
xdebug.auto_trace=0
xdebug.trace_format=11
--FILE--
<?php
$string='/tmp/' . uniqid('xdt',TRUE);
$tf = xdebug_start_trace($string);

class testClass
{
	public static $a;
	private static $b;
	protected static $c;
	const CONSTANT="testClass Name";

	static function test()
	{
		self::$a = 98;
		self::$b = 4;
		self::$b++;
		self::$b=--self::$b;
		self::$b -= 8;
		self::$b *= -0.5;
		self::$b <<= 1;
		self::$c = self::$b / 32;
		self::$a = & self::$b;
		self::$a='Test';
		self::$a.=' it!';
	}
}

testClass::test();
testClass::$a="Nope, false...";
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
{"lvl":1,"aid":3,"atp":2,"tme":%d.%d,"mem":%d,"nme":"$tf","fle":"%sodb_assignment_class.php","lne":3}
{"lvl":2,"aid":4,"tme":%d.%d,"mem":%d,"atp":0,"nme":"testClass::test","ext":1,"ftp":2,"fle":"%sodb_assignment_class.php","lne":28}
{"lvl":2,"aid":5,"atp":2,"tme":%d.%d,"mem":%d,"nme":"self::a","fle":"%sodb_assignment_class.php","lne":14}
{"lvl":2,"aid":6,"atp":2,"tme":%d.%d,"mem":%d,"nme":"self::b","fle":"%sodb_assignment_class.php","lne":15}
{"lvl":2,"aid":7,"atp":2,"tme":%d.%d,"mem":%d,"nme":"self::b","fle":"%sodb_assignment_class.php","lne":16}
{"lvl":2,"aid":8,"atp":2,"tme":%d.%d,"mem":%d,"nme":"self::b","fle":"%sodb_assignment_class.php","lne":17}
{"lvl":2,"aid":9,"atp":2,"tme":%d.%d,"mem":%d,"nme":"self::b","fle":"%sodb_assignment_class.php","lne":17}
{"lvl":2,"aid":10,"atp":2,"tme":%d.%d,"mem":%d,"nme":"self::b","fle":"%sodb_assignment_class.php","lne":18}
{"lvl":2,"aid":11,"atp":2,"tme":%d.%d,"mem":%d,"nme":"self::b","fle":"%sodb_assignment_class.php","lne":19}
{"lvl":2,"aid":12,"atp":2,"tme":%d.%d,"mem":%d,"nme":"self::b","fle":"%sodb_assignment_class.php","lne":20}
{"lvl":2,"aid":13,"atp":2,"tme":%d.%d,"mem":%d,"nme":"self::c","fle":"%sodb_assignment_class.php","lne":21}
{"lvl":2,"aid":14,"atp":2,"tme":%d.%d,"mem":%d,"nme":"self::a","fle":"%sodb_assignment_class.php","lne":22}
{"lvl":2,"aid":15,"atp":2,"tme":%d.%d,"mem":%d,"nme":"self::a","fle":"%sodb_assignment_class.php","lne":23}
{"lvl":2,"aid":16,"atp":2,"tme":%d.%d,"mem":%d,"nme":"self::a","fle":"%sodb_assignment_class.php","lne":24}
{"lvl":2,"aid":4,"tme":%d.%d,"mem":%d,"atp":1}
{"lvl":1,"aid":17,"atp":2,"tme":%d.%d,"mem":%d,"nme":"testClass::a","fle":"%sodb_assignment_class.php","lne":29}
{"lvl":2,"aid":18,"tme":%d.%d,"mem":%d,"atp":0,"nme":"xdebug_stop_trace","ext":0,"ftp":1,"fle":"%sodb_assignment_class.php","lne":30}
{"aid":3,"atp":2,"id":0,"nme":"$tf","val":{"typ":"string","id":%d,"val":"'/tmp/xdt%s.%d.xt'"}}
{"aid":4,"atp":0,"obj":{"typ":"class","nme":"testClass","cid":%d,"val":[{"mod":"public static","nme":"a","val":{"typ":"NULL","id":%d}},{"mod":"private static","nme":"b","val":{"typ":"NULL","id":%d}},{"mod":"protected static","nme":"c","val":{"typ":"NULL","id":%d}},{"mod":"public const","nme":"CONSTANT","val":{"typ":"string","id":%d,"val":"'testClass Name'"}}]},"arg":[]}
{"aid":5,"atp":2,"id":%d,"nme":"self::a","val":{"typ":"int","id":%d,"val":98}}
{"aid":6,"atp":2,"id":%d,"nme":"self::b","val":{"typ":"int","id":%d,"val":4}}
{"aid":7,"atp":2,"id":%d,"nme":"self::b","val":{"typ":"int","id":%d,"val":5}}
{"aid":8,"atp":2,"id":%d,"nme":"self::b","val":{"typ":"int","id":%d,"val":4}}
{"aid":9,"atp":2,"id":%d,"nme":"self::b","val":{"typ":"int","id":%d,"val":4}}
{"aid":10,"atp":2,"id":%d,"nme":"self::b","val":{"typ":"int","id":%d,"val":-4}}
{"aid":11,"atp":2,"id":%d,"nme":"self::b","val":{"typ":"int","id":%d,"val":2}}
{"aid":12,"atp":2,"id":%d,"nme":"self::b","val":{"typ":"int","id":%d,"val":4}}
{"aid":13,"atp":2,"id":%d,"nme":"self::c","val":{"typ":"double","id":%d,"val":0.125}}
{"aid":14,"atp":2,"id":%d,"nme":"self::a","val":{"typ":"int","id":%d,"val":4}}
{"aid":15,"atp":2,"id":%d,"nme":"self::a","val":{"typ":"string","id":%d,"val":"'Test'"}}
{"aid":16,"atp":2,"id":%d,"nme":"self::a","val":{"typ":"string","id":%d,"val":"'Test it!'"}}
{"aid":4,"atp":1,"val":{"typ":"NULL","id":%d}}
{"aid":17,"atp":2,"id":%d,"nme":"testClass::a","val":{"typ":"string","id":%d,"val":"'Nope, false...'"}}
{"aid":18,"atp":0,"obj":{"typ":"NULL","id":0},"arg":[]}
