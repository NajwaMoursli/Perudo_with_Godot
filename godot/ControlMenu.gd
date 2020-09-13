extends Control

var networkThread
var socket

# Called when the node enters the scene tree for the first time.
func _ready():
  createNetworkThread()
  pass

func createNetworkThread():
  networkThread=Thread.new()
  networkThread.start(self, "_thread_network_function", "networkThread")

func _thread_network_function(userdata):
  var done=false
  socket = PacketPeerUDP.new()
  if (socket.listen(global.myPort,global.myIpAddress) != OK):
    print("An error occurred listening on ", global.myIpAddress, ":", global.myPort)
    done = true;
  else:
    print("Listening on ", global.myIpAddress, ":", global.myPort)
  while (done!=true):
    if(socket.get_available_packet_count() > 0):
      var data = socket.get_packet().get_string_from_ascii()
      print("Data received: ",data)
      global.controlGameNode._networkMessage(data)

# Called every frame. 'delta' is the elapsed time since the previous frame.
#func _process(delta):
#	pass

func _on_ButtonJouer_pressed():
  var connectMessage="C " + str(global.myIpAddress) + " " + str(global.myPort) + " " + $CanvasLayer/Username.text
  $CanvasLayer/Username.text = "Pseudonyme"
  sendMessageToServer(connectMessage)
  var root=get_tree().get_root()
  var myself=root.get_child(1)
  print (root,myself)
  root.remove_child(myself)
  root.add_child(global.controlGameNode)

func _on_ButtonOptions_pressed():
  var root=get_tree().get_root()
  var myself=root.get_child(1)
  print (root,myself)
  root.remove_child(myself)
  root.add_child(global.controlOptionsNode)

func _on_ButtonTestReseau_pressed():
  print ("Sending UDP test data to ",global.serverIpAddress,":",global.serverPort)
  sendMessageToServer("This is a UDP test.")

func sendMessage(ipaddr, portno, mess):
  socket.set_dest_address(ipaddr, portno)
  var message = mess + " " + str(global.direction) + "\n"
  socket.put_packet(message.to_ascii())
  
func sendMessageToServer(mess):
  sendMessage(global.serverIpAddress, global.serverPort, mess)
  
func _on_Username_focus_entered():
  $CanvasLayer/Username.clear()
