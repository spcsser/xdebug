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
	public $a;
	private $b;
	protected $c;
	public $text;

	function __construct()
	{
		$this->a = 98;
		$this->b = 4;
		$this->b++;
		$this->b=--$this->b;
		$this->b -= 8;
		$this->b *= -0.5;
		$this->b <<= 1;
		$this->c = $this->b / 32;
		$this->a = & $this->b;
		$this->text='Test';
		$this->text.=' it!';
	}
}

$a = new testClass;
$a->text="Nope, false...";
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
{"lvl":1,"aid":3,"atp":2,"tme":%d.%d,"mem":%d,"nme":"$tf","fle":"%sodb_assignment_object.php","lne":3}
{"lvl":2,"aid":4,"tme":%d.%d,"mem":%d,"atp":0,"nme":"testClass->__construct","ext":1,"ftp":3,"fle":"%sodb_assignment_object.php","lne":28}
{"lvl":2,"aid":5,"atp":2,"tme":%d.%d,"mem":%d,"nme":"$this->a","fle":"%sodb_assignment_object.php","lne":14}
{"lvl":2,"aid":6,"atp":2,"tme":%d.%d,"mem":%d,"nme":"$this->b","fle":"%sodb_assignment_object.php","lne":15}
{"lvl":2,"aid":7,"atp":2,"tme":%d.%d,"mem":%d,"nme":"$this->b","fle":"%sodb_assignment_object.php","lne":16}
{"lvl":2,"aid":8,"atp":2,"tme":%d.%d,"mem":%d,"nme":"$this->b","fle":"%sodb_assignment_object.php","lne":17}
{"lvl":2,"aid":9,"atp":2,"tme":%d.%d,"mem":%d,"nme":"$this->b","fle":"%sodb_assignment_object.php","lne":17}
{"lvl":2,"aid":10,"atp":2,"tme":%d.%d,"mem":%d,"nme":"$this->b","fle":"%sodb_assignment_object.php","lne":18}
{"lvl":2,"aid":11,"atp":2,"tme":%d.%d,"mem":%d,"nme":"$this->b","fle":"%sodb_assignment_object.php","lne":19}
{"lvl":2,"aid":12,"atp":2,"tme":%d.%d,"mem":%d,"nme":"$this->b","fle":"%sodb_assignment_object.php","lne":20}
{"lvl":2,"aid":13,"atp":2,"tme":%d.%d,"mem":%d,"nme":"$this->c","fle":"%sodb_assignment_object.php","lne":21}
{"lvl":2,"aid":14,"atp":2,"tme":%d.%d,"mem":%d,"nme":"$this->a","fle":"%sodb_assignment_object.php","lne":22}
{"lvl":2,"aid":15,"atp":2,"tme":%d.%d,"mem":%d,"nme":"$this->text","fle":"%sodb_assignment_object.php","lne":23}
{"lvl":2,"aid":16,"atp":2,"tme":%d.%d,"mem":%d,"nme":"$this->text","fle":"%sodb_assignment_object.php","lne":24}
{"lvl":2,"aid":4,"tme":%d.%d,"mem":%d,"atp":1}
{"lvl":1,"aid":17,"atp":2,"tme":%d.%d,"mem":%d,"nme":"$a","fle":"%sodb_assignment_object.php","lne":28}
{"lvl":1,"aid":18,"atp":2,"tme":%d.%d,"mem":%d,"nme":"$a->text","fle":"%sodb_assignment_object.php","lne":29}
{"lvl":2,"aid":19,"tme":%d.%d,"mem":%d,"atp":0,"nme":"xdebug_stop_trace","ext":0,"ftp":1,"fle":"%sodb_assignment_object.php","lne":30}
{"aid":3,"atp":2,"id":0,"nme":"$tf","val":{"typ":"string","id":%d,"val":"'/tmp/xdt%s.%d.xt'"}}
{"aid":4,"atp":0,"obj":{"typ":"object","nme":"testClass","id":%d,"cid":%d,"val":[{"mod":"public","nme":"a","val":{"typ":"NULL","id":%d}},{"mod":"private","nme":"b","val":{"typ":"NULL","id":%d}},{"mod":"protected","nme":"c","val":{"typ":"NULL","id":%d}},{"mod":"public","nme":"text","val":{"typ":"NULL","id":%d}}]},"arg":[]}
{"aid":5,"atp":2,"id":%d,"nme":"$this->a","val":{"typ":"int","id":%d,"val":98}}
{"aid":6,"atp":2,"id":%d,"nme":"$this->b","val":{"typ":"int","id":%d,"val":4}}
{"aid":7,"atp":2,"id":%d,"nme":"$this->b","val":{"typ":"int","id":%d,"val":5}}
{"aid":8,"atp":2,"id":%d,"nme":"$this->b","val":{"typ":"int","id":%d,"val":4}}
{"aid":9,"atp":2,"id":%d,"nme":"$this->b","val":{"typ":"int","id":%d,"val":4}}
{"aid":10,"atp":2,"id":%d,"nme":"$this->b","val":{"typ":"int","id":%d,"val":-4}}
{"aid":11,"atp":2,"id":%d,"nme":"$this->b","val":{"typ":"int","id":%d,"val":2}}
{"aid":12,"atp":2,"id":%d,"nme":"$this->b","val":{"typ":"int","id":%d,"val":4}}
{"aid":13,"atp":2,"id":%d,"nme":"$this->c","val":{"typ":"double","id":%d,"val":0.125}}
{"aid":14,"atp":2,"id":%d,"nme":"$this->a","val":{"typ":"int","id":%d,"val":4}}
{"aid":15,"atp":2,"id":%d,"nme":"$this->text","val":{"typ":"string","id":%d,"val":"'Test'"}}
{"aid":16,"atp":2,"id":%d,"nme":"$this->text","val":{"typ":"string","id":%d,"val":"'Test it!'"}}
{"aid":4,"atp":1,"val":{"typ":"NULL","id":%d}}
{"aid":17,"atp":2,"id":0,"nme":"$a","val":{"typ":"object","nme":"testClass","id":%d,"cid":%d,"val":[{"mod":"public","nme":"a","val":{"typ":"int","id":%d,"val":4}},{"mod":"private","nme":"b","val":{"id":%d}},{"mod":"protected","nme":"c","val":{"typ":"double","id":%d,"val":0.125}},{"mod":"public","nme":"text","val":{"typ":"string","id":%d,"val":"'Test it!'"}}]}}
{"aid":18,"atp":2,"id":%d,"nme":"$a->text","val":{"typ":"string","id":%d,"val":"'Nope, false...'"}}
{"aid":19,"atp":0,"obj":{"typ":"NULL","id":0},"arg":[]}
