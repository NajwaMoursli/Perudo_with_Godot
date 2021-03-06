extends Control

# Called when the node enters the scene tree for the first time.
func _ready():
  global.controlMenuNode=load("res://ControlMenu.tscn").instance()
  global.controlGameNode=load("res://ControlGame.tscn").instance()
  global.controlOptionsNode=load("res://ControlOptions.tscn").instance()
  global.myIpAddress=str(IP.get_local_addresses()[2])
  match global.myIpAddress:
    "192.168.1.75":
      global.direction=0
    "192.168.1.76":
      global.direction=1
    "192.168.1.77":
      global.direction=2
    "192.168.1.78":
      global.direction=3
    _:
      global.direction=3
  
  print ("controlMenuNode=",global.controlMenuNode)
  print ("controlGameNode=",global.controlGameNode)
  print ("controlOptionsNode=",global.controlOptionsNode)
  print ("IP=",global.myIpAddress)
  print ("Direction=",global.direction)
  print ("controlSplashNode=",self)
  print ("controlSplash _ready")

# Called every frame. 'delta' is the elapsed time since the previous frame.
#func _process(delta):
#	pass

func _on_ButtonVersMenu_pressed():
  var root=get_tree().get_root()
  var myself=root.get_child(1)
  print (root,myself)
  root.remove_child(myself)
  root.add_child(global.controlMenuNode)
