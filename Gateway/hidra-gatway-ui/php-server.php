<?php

// Configurações iniciais
$NUM_MODULOS = 6;
$NUM_SENSORES = 5;

// Simula nomes de sensores e módulos
$sensorTypes = ['NTC', 'COR', 'SUN'];
$moduleNames = ['rio azul 2', 'rio mais verde', 'verde vivo', 'hidra nascente', 'rio de março', 'lagoa azul'];

$serverStatus = 0; // 1 = ligado, 0 = desligado
$wifiStatus = 1; // 1 = ligado, 0 = desligado

header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: GET, POST, DELETE');
header('Access-Control-Allow-Headers: Content-Type');
header('Content-Type: application/json');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

// Utilitários
function randomDateTime()
{
    return [
        "date" => date("d/m/Y"),
        "time" => date("H:i:s")
    ];
}
function randSensor($id)
{
    $type = ["NTC", "COR", "SUN"][array_rand([0, 1, 2])];
    if ($type === "COR") {
        return [
            "id" => $id,
            "type" => $type,
            "value-1" => rand(100, 999),
            "value-2" => rand(100, 999),
            "value-3" => rand(100, 999),
            "value-4" => rand(100, 999)
        ];
    } else {
        return [
            "id" => $id,
            "type" => $type,
            "value" => round(rand(100, 4000) / 1.11, 2)
        ];
    }
}
function getModules($num)
{
    global $moduleNames;
    $mods = [];
    for ($i = 0; $i < $num; $i++) {
        $mods[] = [
            "id" => $i,
            "name" => $moduleNames[$i % count($moduleNames)],
            "recieve-date" => date("d/m/Y"),
            "recieve-time" => date("H:i:s"),
            "address" => "0x" . dechex(rand(0, 255)) . dechex(rand(0, 255)),
            "bat" => rand(20, 100)
        ];
    }
    return $mods;
}

// Roteamento
$uri = $_SERVER['REQUEST_URI'];
$method = $_SERVER['REQUEST_METHOD'];

if ($method === 'GET') {
    if ($uri === '/') {
        header('Content-Type: text/plain');
        readfile('../README.md');
    } elseif ($uri === '/config') {
        sleep(2);
        echo json_encode(array_merge(randomDateTime(), ["address" => "0xaa"]));
    } elseif ($uri === '/wifi/status') {
        echo json_encode([
            "ssid" => "wifi-teste",
            "rssi" => "-30",
            "ip" => "192.168.4.1",
            "status" => $wifiStatus
        ]);
    } elseif ($uri === '/wifi/networks') {
        echo json_encode([
            "saved" => [
                ["id" => 0, "ssid" => "rede 1", "pass" => "********"],
                ["id" => 1, "ssid" => "rede 2", "pass" => "********"],
                ["id" => 2, "ssid" => "rede D", "pass" => "********"],
                ["id" => 3, "ssid" => "rede 3", "pass" => "********"],
                ["id" => 4, "ssid" => "rede F", "pass" => "********"],
            ],
            "near" => [
                ["id" => 0, "ssid" => "rede A", "rssi" => "-80"],
                ["id" => 1, "ssid" => "rede B", "rssi" => "-90"],
                ["id" => 2, "ssid" => "rede C", "rssi" => "-96"],
                ["id" => 3, "ssid" => "rede D", "rssi" => "-70"],
                ["id" => 4, "ssid" => "rede E", "rssi" => "-85"],
                ["id" => 5, "ssid" => "rede F", "rssi" => "-60"],
                ["id" => 6, "ssid" => "rede G", "rssi" => "-75"],
                ["id" => 7, "ssid" => "rede H", "rssi" => "-67"],
                ["id" => 8, "ssid" => "rede I", "rssi" => "-88"],
                ["id" => 9, "ssid" => "rede J", "rssi" => "-92"],
                ["id" => 10, "ssid" => "rede K", "rssi" => "-43"],
                ["id" => 11, "ssid" => "rede L", "rssi" => "-78"],
                ["id" => 12, "ssid" => "rede M", "rssi" => "-66"],
                ["id" => 13, "ssid" => "rede N", "rssi" => "-82"],
                ["id" => 14, "ssid" => "rede O", "rssi" => "-59"]
            ]

        ]);
    } elseif ($uri === '/server/config') {
        echo json_encode([
            "ssid" => "hidra",
            "pass" => "hidra1234",
            "ip" => "192.168.4.1",
            "status" => $serverStatus
        ]);
    } elseif ($uri === '/modules') {
        sleep(1);
        echo json_encode(["modules" => getModules($GLOBALS['NUM_MODULOS'])]);
    } elseif (preg_match('#^/modules/(\d+)/date$#', $uri, $matches)) {
        $id = intval($matches[1]);
        echo json_encode([
            "id" => $id,
            "name" => $moduleNames[$id % count($moduleNames)],
            "recieve-date" => date("d/m/Y"),
            "recieve-time" => date("H:i:s"),
            "module-date" => date("d/m/Y"),
            "module-time" => date("H:i:s"),
            "address" => "0x" . dechex(rand(0, 255)) . dechex(rand(0, 255)),
            "bat" => rand(20, 100),
            "sensors" => array_map(fn($i) => randSensor($i), range(0, $GLOBALS['NUM_SENSORES'] - 1))
        ]);
    } else {
        http_response_code(404);
        echo json_encode(["error" => "GET route not found"]);
    }
} elseif ($method === 'POST') {
    $input = json_decode(file_get_contents("php://input"), true);

    if ($uri === '/wifi/connect') {
        sleep(1);
        echo json_encode(["status" => "connecting", "input" => $input]);
    } elseif ($uri === '/wifi/toggle') {
        $wifiStatus = $input["status"];
        echo json_encode(["status" => "wifi toggled", "input" => $input]);
    } elseif ($uri === '/server/toggle') {
        $serverStatus = $input["status"];
        echo json_encode(["status" => "server toggled", "input" => $input]);
    } elseif ($uri === '/config/time') {
        echo json_encode(["status" => "time updated", "input" => $input]);
    } elseif ($uri === '/server/config') {
        echo json_encode(["status" => "server config updated", "input" => $input]);
    } elseif ($uri === '/modules/new') {
        echo json_encode(["status" => "module added", "input" => $input]);
    } elseif (preg_match('#^/modules/(\d+)/config$#', $uri, $matches)) {
        echo json_encode(["status" => "module config updated", "id" => $matches[1], "input" => $input]);
    } else {
        http_response_code(404);
        echo json_encode(["error" => "POST route not found"]);
    }
} elseif ($method === 'DELETE') {
    $input = json_decode(file_get_contents("php://input"), true);

    if (preg_match('#^/wifi/\d+$#', $uri)) {
        echo json_encode(["status" => "wifi deleted", "input" => $input]);
    } elseif (preg_match('#^/modules/\d+$#', $uri)) {
        echo json_encode(["status" => "module deleted", "input" => $input]);
    } else {
        http_response_code(404);
        echo json_encode(["error" => "DELETE route not found"]);
    }
} else {
    http_response_code(405);
    echo json_encode(["error" => "Method not allowed"]);
}
