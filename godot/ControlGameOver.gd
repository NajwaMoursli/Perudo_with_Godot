extends Node2D


func _ready():
  pass 


#Bouton pour sortir du jeu 
func _on_exit_pressed():
  get_tree().quit()


#Bouton pour jouer à nouveau 
func _on_playAgain_pressed():
  var root=get_tree().get_root()
  var myself=root.get_child(1)
  print (root,myself)
  root.remove_child(myself)
  root.add_child(global.controlSplashNode)
#  get_tree().change_scene("res://ControlSplash.tscn")
