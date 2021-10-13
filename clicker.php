<?php
error_reporting(0);

// parse uri arguments
$query = parse_url($_SERVER["REQUEST_URI"], PHP_URL_QUERY);
parse_str($query, $parsed_query);

// arguments whatever
$save = $parsed_query['save'];  
$load = $parsed_query['load'];

function random_string($n)
{
    $characters = '0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ';
    $random_string = '';

    for ($i = 0; $i < $n; $i++) 
    {
        $index = rand(0, strlen($characters) - 1);
        $random_string .= $characters[$index];
    }

    return $random_string;
}

function get_ip()
{
    if (!empty($_SERVER['HTTP_CLIENT_IP']))
    {
      $ip=$_SERVER['HTTP_CLIENT_IP'];
    }
    elseif (!empty($_SERVER['HTTP_X_FORWARDED_FOR']))
    {
      $ip=$_SERVER['HTTP_X_FORWARDED_FOR'];
    }
    else
    {
      $ip=$_SERVER['REMOTE_ADDR'];
    }
    return $ip;
}

// generate key
$generate = $parsed_query['generate'];

if (isset($generate))
{
	if ($generate == "lilyiscool")
	{
	    $code_var = random_string(48);
	    mysqli_query($mysqli, "INSERT INTO `clicker_users` (`code`) VALUES ('{$code_var}')");

	    $url = "https://lilyiscool.com/clicker.php?download=".$code_var;

	    echo $url;	
	}
}

// login api
$login = $parsed_query['login'];

if (isset($login))
{
	if ($login !== "")
	{
		$ip_addr = get_ip();

		$ip_query = mysqli_query($mysqli, "SELECT * FROM `clicker_users` WHERE `ip` ='{$ip_addr}'");
    	$ip_fetch = mysqli_fetch_array($ip_query);

    	$hwid_query = mysqli_query($mysqli, "SELECT * FROM `clicker_users` WHERE `hwid` ='{$login}'");
	    $hwid_fetch = mysqli_fetch_array($hwid_query);

    	if ($ip_fetch !== NULL)
    	{
	    	if ($hwid_fetch !== NULL)
	    	{
	    		$data_json = array('status' => "success", 'username' => $hwid_fetch['user']);
	    		$json_encoded = trim(json_encode($data_json));
	    		echo base64_encode($json_encoded);
	    	}
	    	else
	    	{	
	    		if ($ip_fetch['hwid'] == "")
	    		{
					$data_json = array('status' => "failure", 'reason' => "not_registered");
	    		    $json_encoded = trim(json_encode($data_json));
	    		    echo base64_encode($json_encoded);
				}
				else
				{
					$data_json = array('status' => "failure", 'reason' => "hwid_error");
	    		    $json_encoded = trim(json_encode($data_json));
	    		    echo base64_encode($json_encoded);
				}
			}
		}
		else
		{
			if ($hwid_fetch !== NULL)
	    	{
	    		$data_json = array('status' => "success", 'username' => $hwid_fetch['user']);
	    		$json_encoded = trim(json_encode($data_json));
	    		echo base64_encode($json_encoded);
	    	}
	    	else 
	    	{
				$data_json = array('status' => "failure", 'reason' => "invalid_user");
	    		$json_encoded = trim(json_encode($data_json));
	    		echo base64_encode($json_encoded);
			}
		}
	}
}

// register api
$register = $parsed_query['register'];
$hwid = $parsed_query['hwid'];

if (isset($register))
{
	if ($register !== "")
	{
		if ($hwid !== "") 
		{
        	$hwid_query = mysqli_query($mysqli, "SELECT * FROM `clicker_users` WHERE `hwid` ='{$hwid}'");
        	$hwid_fetch = mysqli_fetch_array($hwid_query);

        	$user_query = mysqli_query($mysqli, "SELECT * FROM `clicker_users` WHERE `user` ='{$register}'");
        	$user_fetch = mysqli_fetch_array($user_query);

        	if ($hwid_fetch !== NULL || $user_fetch !== NULL)
        	{
        		$data_json = array('status' => "failure", 'reason' => "already_registered");
	    		$json_encoded = trim(json_encode($data_json));
	    		
	    		echo base64_encode($json_encoded);
        	}
        	else 
        	{
				$ip_addr = get_ip();

				mysqli_query($mysqli, "UPDATE `clicker_users` SET `user`='{$register}' WHERE `ip`='{$ip_addr}'");
				mysqli_query($mysqli, "UPDATE `clicker_users` SET `hwid`='{$hwid}' WHERE `ip`='{$ip_addr}'");

				$data_json = array('status' => "success");
	    		$json_encoded = trim(json_encode($data_json));

	    		echo base64_encode($json_encoded);
			}
		}
	}
}

// download clicker and store ip
$download = $parsed_query['download'];

if (isset($download)) 
{
	if ($download !== "") 
	{
		$fetch_key = mysqli_query($mysqli, "SELECT `code` FROM `clicker_users` WHERE `code` ='{$download}'");
        $run_fetch_key = mysqli_fetch_all($fetch_key, MYSQLI_ASSOC);

        if ($run_fetch_key) 
		{
			$ip_addr = get_ip();

			mysqli_query($mysqli, "UPDATE `clicker_users` SET `ip`='{$ip_addr}' WHERE `code`='{$download}'");
			$name = random_string(20);

			header('Content-type: application/x-dosexec');
			header('Content-Disposition: attachment; filename="'.$name.'.exe"');

			readfile("https://lilyiscool.com/bin/private.exe");
		}
		else
		{
			echo "invalid download key";
		}
	}
}

// save cloud config
$save = $parsed_query['save'];
$data = $parsed_query['data'];

if (isset($save)) 
{
	if ($save !== "")
	{
		if ($data !== "") 
		{
			mysqli_query($mysqli, "UPDATE `clicker_users` SET `config`='{$data}' WHERE `hwid` = '{$save}'");

			$data_json = array('status' => "success");
	    	$json_encoded = trim(json_encode($data_json));
	    	echo base64_encode($json_encoded);
		}
	}
}

// load cloud config
$load = $parsed_query['load'];

if (isset($load)) 
{
	if ($load !== "")
	{
		$config_query = mysqli_query($mysqli, "SELECT * FROM `clicker_users` WHERE `hwid` = '{$load}'");
	    $config_array = mysqli_fetch_array($config_query);

	    echo base64_encode($config_array['config']);
	}
}

?>