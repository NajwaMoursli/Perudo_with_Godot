extends Control

var betSent = false
var currentPlayer
var diceNode
var diceNum = [5, 5, 5, 5]
var diceValue = [[0,0,0,0,0], [0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0]]
var endGame = [false, false, false, false]
var names = ["", "", "", ""]
var colors = ["bleu", "jaune", "orange", "rouge", "vert", "violet"]
var labels
const numPlayer = 4
var previousBetDice = 0
var previousBetValue = 0
var isPalifico = [false, false, false, false]
var palifico = false
var startPlayer
var firstTour
var rng = RandomNumberGenerator.new()

func _ready():
  rng.randomize()
  $Spatial/Countdown.hide()
  $Spatial/DiceLineEdit.hide()
  $Spatial/ValueLineEdit.hide()
  $Spatial/BetButton.hide()
  $Spatial/DudoButton.hide()
  $Spatial/NorthLabel.hide()
  $Spatial/EastLabel.hide()
  $Spatial/WestLabel.hide()
  $Spatial/SouthLabel.hide()
  $Spatial/WrongSprite.hide()
  $Spatial/WrongLabel.hide()
  $Spatial/BetLabel.hide()
  $Spatial/ConnectedPopupPanel.popup(Rect2(0,600,200,100))
  $Spatial/ConnectedPopupPanel.hide()
  labels = [$Spatial/NorthLabel, $Spatial/EastLabel, $Spatial/SouthLabel, $Spatial/WestLabel]
  diceNode = [[$Spatial/NorthDice1,$Spatial/NorthDice2,$Spatial/NorthDice3,$Spatial/NorthDice4,$Spatial/NorthDice5],[$Spatial/EastDice1,$Spatial/EastDice2,$Spatial/EastDice3,$Spatial/EastDice4,$Spatial/EastDice5],[$Spatial/SouthDice1,$Spatial/SouthDice2,$Spatial/SouthDice3,$Spatial/SouthDice4,$Spatial/SouthDice5],[$Spatial/WestDice1,$Spatial/WestDice2,$Spatial/WestDice3,$Spatial/WestDice4,$Spatial/WestDice5]]


# Called every frame. 'delta' is the elapsed time since the previous frame.
#func _process(delta):
#  pass

func _networkMessage(mess):
  print ("_networkMessage=",mess)
  match mess[0]:
    "J":
      newPlayer(mess)
    'F':
      end(mess)
    'P':
      printBet(mess)
      if firstTour:
        firstTour = false
    'O':
      palifico = true
      isPalifico[int(mess[2])] = true
      print("Palifico turn")
    'L':
      loseDice(mess)
    'M':
      dudo(mess)
    'W':
      win(mess)
      endRound()
    'Q':  
      loseGame(mess)
    'R':
      reveal(mess)
    'S':
      startRound(mess)
    'T':
      turn(mess)
    _:
      print("[_networkMessage] Incorrect message: ", mess)

func _on_ButtonMenu_pressed():
  var root=get_tree().get_root()
  var myself=root.get_child(1)
  print (root,myself)
  root.remove_child(myself)
  root.add_child(global.controlMenuNode)
  
func startGame():
  print("StartGame")
  $Spatial/ConnectedPopupPanel.hide()
  $Spatial/NorthLabel.text = names[0]
  $Spatial/EastLabel.text = names[1]
  $Spatial/SouthLabel.text = names[2]
  $Spatial/WestLabel.text = names[3]
  $Spatial/NorthLabel.show()
  $Spatial/EastLabel.show()
  $Spatial/WestLabel.show()
  $Spatial/SouthLabel.show()
  $Spatial/DiceLineEdit.show()
  $Spatial/DiceLineEdit.editable = false
  $Spatial/ValueLineEdit.show()
  $Spatial/ValueLineEdit.editable = false
  $Spatial/BetButton.show()
  $Spatial/BetButton.disabled = true
  $Spatial/DudoButton.show()
  $Spatial/DudoButton.disabled = true
  $Spatial/BetLabel.show()

func startRound(mess):
  print("StartRound")
  startPlayer = int(mess[2])
  print("Startplayer: ", startPlayer)
  currentPlayer = startPlayer
  print("Currentplayer: ", currentPlayer)
  for player in range(0, numPlayer):
    for dice in range(0,5):
      showDiceSprite(player, dice, 0)
    if not endGame[player]:
      labels[player].text = names[player]
  shake()
  showDices(global.direction)
  firstTour = true
  previousBetDice = 0
  previousBetValue = 0
  $Spatial/DiceLineEdit.text = "Nombre de dés"
  $Spatial/ValueLineEdit.text = "Valeur"
  $Spatial/DiceLineEdit.editable = true
  $Spatial/BetLabel.text = "Nouvelle manche !"
  if palifico:
    $Spatial/ValueLineEdit.editable = isPalifico[global.direction]
    $Spatial/BetLabel.text += "Attention Palifico !"
  else:
    $Spatial/ValueLineEdit.editable = true
  $Spatial/BetLabel.text += " " + names[currentPlayer] + " commence.\n"

func newPlayer(mess):
  var messSplit = mess.trim_prefix("J ").trim_suffix("\n").split(" ", false)
  var n = messSplit.size()
  names = messSplit
  $Spatial/ConnectedPopupPanel/ConnectedLabel.text = str(n) +"/4 joueur(s) connecté(s) :\n"
  $Spatial/ConnectedPopupPanel/ConnectedLabel.text += messSplit.join(" connecté\n")
  $Spatial/ConnectedPopupPanel/ConnectedLabel.text += " connecté\n"
  $Spatial/ConnectedPopupPanel.show()
  if (n == 4):
    startGame()

func turn(mess):
  currentPlayer = int(mess[2])
  print("Currentplayer: ", currentPlayer)
  labels[currentPlayer].text = names[currentPlayer] + " [Guess]"
  var previousPlayer = currentPlayer - 1
  labels[previousPlayer].text = names[previousPlayer]
  if currentPlayer == global.direction:
    betSent = false
    yourTurn()

func yourTurn():
  print("yourTurn")
  $Spatial/Countdown.show()
  $Spatial/DiceLineEdit.text = "Nombre de dés"
  $Spatial/ValueLineEdit.text = "Valeur"
  $Spatial/DiceLineEdit.editable = true
  if palifico:
    $Spatial/ValueLineEdit.editable = isPalifico[global.direction]
  else:
    $Spatial/ValueLineEdit.editable = true
  $Spatial/BetButton.disabled = false
  if not firstTour:
    $Spatial/DudoButton.disabled = false
  $Timer.start(30)
  while not $Timer.is_stopped() or not betSent:
    $Spatial/Countdown.text = "Temps restant: " + str(int($Timer.get_time_left()))
  $Spatial/Countdown.hide()
  $Spatial/WrongSprite.hide()
  $Spatial/WrongLabel.hide()
  $Spatial/WrongLabel.set_text("")
  $Spatial/DiceLineEdit.editable = false
  $Spatial/ValueLineEdit.editable = false
  $Spatial/DiceLineEdit.clear()
  $Spatial/ValueLineEdit.clear()
  $Spatial/BetButton.disabled = true
  $Spatial/DudoButton.disabled = true

func shake():
  print("Shake")
  var mess = "D " + str(global.direction)
  for i in range(0,diceNum[global.direction]):
    diceValue[global.direction][i] = generateRandomDice()
    mess += " " + str(diceValue[global.direction][i])
  for i in range(diceNum[global.direction], 5):
    mess += " 0"
  global.controlMenuNode.sendMessageToServer(mess)

  
func showAllDices():
  print("showAllDices")
  for player in range(0,numPlayer):
    showDices(player)


func showDices(player):
  print("showDices")
  if not endGame[player]:
    for dice in range(0,diceNum[player]):
      print("  player ",player,",value ", diceValue[player][dice])
      showDiceSprite(player, dice, diceValue[player][dice])
    for dice in range(diceNum[player], 5):
      print("  player ",player,",value ", 0)
      showDiceSprite(player, dice, 0)
  else:
    for dice in range(0,5):
      print("  player ",player,",value ", 0)
      showDiceSprite(player, dice, 0)


func showDiceSprite(player, i, value):
  if 0 < value and value < 7:
    # create a new image texture that will be used as a tile texture
    print("Sprite: player ",player,",value ", value)
    var texture=ImageTexture.new()
    print("res://assets/d"+str(value)+"_"+colors[player] + ".png")
    texture.load("res://assets/d"+str(value)+"_"+colors[player] + ".png")
    # and perform the assignment to the surface_material
    diceNode[player][i].set_texture(texture)
    diceNode[player][i].show()
  elif value == 0:
    diceNode[player][i].set_texture(null)
    diceNode[player][i].hide()
 

func makeGuess(dice, value):
  print("makeGuess")
  if palifico:
    print("PALIFICO guess")
    if global.direction != startPlayer or not firstTour:
      value = previousBetValue
    if int(dice) > previousBetDice:
      sendBet(value, dice)
    else:
      showWarning("WARNING: the number of dices must be higher than the previous bet.\n")
  else:
    value = int(value)
    dice = int(dice)
    print("dice:", dice,", value:", value, ", previousDice:", previousBetDice, ", previousValue:", previousBetValue)
    if global.direction == startPlayer and firstTour:
      if value == 1:
        showWarning("WARNING: a bet with Paco is forbidden for the first player of a round without Palifico.\n")
      elif (0<value and value <7 and 0<dice):
        sendBet(value, dice)
      else:
        showWarning("WARNING: '0<value<7 and 0<dice' is false.\n")
    else:
      if (0<value and value <7 and 0<dice):
        if (value == previousBetValue and dice > previousBetDice):
          sendBet(value, dice)
        else:
          showWarning("WARNING: 'value == previousBetValue and dice > previousBetDice' is false\n")
          if (previousBetValue != 1 and dice == previousBetDice and value > previousBetValue):
            sendBet(value, dice)
          else:
            showWarning("WARNING: 'previousBetValue != 1 and dice == previousBetDice and value > previousBetValue' is false\n")
            if  (previousBetValue!= 1 and value == 1 and dice > previousBetDice / 2):
              sendBet(value, dice)
            else:
              showWarning("WARNING: 'previousBetValue != 1 and value == 1 and dice > previousBetDice / 2' is false\n")
              if (previousBetValue == 1 and value > 1 and dice > 2 * previousBetDice):
                sendBet(value, dice)
              else:
                showWarning("WARNING: incorrect bet\n")
      else:
        showWarning("WARNING: '0<value<7 and 0<dice' is false.\n")
  $Timer.stop()


func sendBet(value, dice):
  var mess = "P " + str(global.direction) + " " + str(value) + " " + str(dice)
  global.controlMenuNode.sendMessageToServer(mess)
  betSent = true
  firstTour= false
  $Spatial/BetLabel.text += names[global.direction] + " dit " + str(dice) + " dé(s) de valeur " + str(value) + ".\n"


func printBet(mess):
  if not firstTour or currentPlayer != global.direction:
    previousBetValue = int(mess[4])
    previousBetDice = int(mess[6])
    print("Bet P", previousBetValue, previousBetDice)
    $Spatial/BetLabel.text += names[currentPlayer] + " dit " + str(previousBetDice) + " dé(s) de valeur " + str(previousBetValue) + ".\n"


func showWarning(mess):
  $Spatial/WrongLabel.text += mess
  $Spatial/WrongSprite.show()
  $Spatial/WrongLabel.show()


func dudo(mess):
  print("Someone 'dudo'")
  var player = int(mess[2])
  labels[player].text = names[player] + " [Dudo]"

func win(mess):
  print("Bravo !")
  var player = int(mess[2])
  labels[player].text = names[player] + " [Win]"
  
func loseDice(mess):
  print("lose a dice")
  var player = int(mess[2])
  labels[player].text = names[player] + " [-1 Dice]"
  diceNum[player] -= 1
  if diceNum[player] == 1:
    isPalifico[player] = true

func loseGame(mess):
  print("lose -> game over")
  var player = int(mess[2])
  labels[player].text = names[player] + " [0 Dice]"
  diceNum[player] -= 1
  endGame[player] = true

func reveal(mess):
  print("reveal")
  var messSplit = mess.trim_prefix("R ").trim_suffix("\n").split(" ", false)
#  print(messSplit)
  var k = 0
  for player in range(0,numPlayer):
    for dice in range(0,diceNum[player]):
      diceValue[player][dice] = int(messSplit[k])
      k += 1
    k += 5 - diceNum[player]
  showAllDices()


func generateRandomDice():
  print("generateRandomDice")
  var value = rng.randi_range(1,6)
  return value


func endRound():
  if palifico:
    palifico = false
  for boolean in isPalifico:
    boolean = false
  firstTour = true

func end(mess):
  print("Fin de la partie. " + mess.substr(2) + " a gagné")
  var root=get_tree().get_root()
  var myself=root.get_child(1)
  print (root,myself)
  root.remove_child(myself)
  root.add_child(global.controlMenuNode)

func _on_DiceLineEdit_focus_entered():
  if currentPlayer == global.direction:
    $Spatial/DiceLineEdit.clear()


func _on_ValueLineEdit_focus_entered():
  if currentPlayer == global.direction:
    $Spatial/ValueLineEdit.clear()


func _on_BetButton_pressed():
  print("BetButton")
  makeGuess($Spatial/DiceLineEdit.text, $Spatial/ValueLineEdit.text)


func _on_Timer_timeout():
  $Timer.stop()
  print("timeout... sending default guess")
  makeGuess(1,1)


func _on_DudoButton_pressed():
  if not firstTour:
    $Timer.stop()
    var mess = "M"
    global.controlMenuNode.sendMessageToServer(mess)
    betSent = true
