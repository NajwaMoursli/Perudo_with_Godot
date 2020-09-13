extends Button



func _ready():

	pass



func _input(event):

	if event is InputEventMouseButton:

		if event.is_pressed():

			print('Play Pressed')

			get_tree().change_scene("res://scenes/game.tscn")
