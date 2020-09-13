extends Node

var mode
var network
var mplayer

var controlMenuNode
var controlGameNode
var controlOptionsNode
var controlSplashNode
var myIpAddress
var direction

const serverIpAddress = "192.168.1.16"
const serverPort = 4242
const myPort = 4000

var previous
var current

func _ready():
  direction=0
  print("global _ready() called")
