import telegram, json, os, requests
from telegram import ReplyKeyboardMarkup
from telegram.ext import Updater, CommandHandler, MessageHandler, Filters, ConversationHandler

TOKEN = "1187705175:AAFkncTHtggO0nYZEBmwLi7969Z5IBGOUzY"

CHOOSING_DEVICE, DEVICE_OPERATION = range(2)

def getMkpCommand(ID):
    r = requests.get("http://localhost:8080/catalog/devices/all")
    if r.status_code != 200:
        print("HTTP Error %s" %r.status_code)
    data = json.loads(r.content)

    if ID == "All":
        command_keyboard = [["On", "Off"], ["Done"]]
        markup_command = ReplyKeyboardMarkup(command_keyboard, resize_keyboard=True, one_time_keyboard=True)
        return markup_command

    resources = next(x["resources"] for x in data["devices"] if x["deviceID"] == ID)

    if "illumination" in resources:
        command_keyboard = [["On", "Off"], ["Done"]]
        markup_command = ReplyKeyboardMarkup(command_keyboard, resize_keyboard=True, one_time_keyboard=True)
    else:
        command_keyboard = []
        for resource in resources:
            command_keyboard.append([resource])
        command_keyboard.append(["Done"])
        markup_command = ReplyKeyboardMarkup(command_keyboard, resize_keyboard=True, one_time_keyboard=True)

    return markup_command

def getMkpDevice():
    r = requests.get("http://localhost:8080/catalog/devices/all")
    if r.status_code != 200:
        print("HTTP Error %s" %r.status_code)
    data = json.loads(r.content)

    IDs = []
    for device in data["devices"]:
        IDs.append(device["deviceID"])

    device_keyboard = []
    for ID in IDs:
        device_keyboard.append([ID])

    device_keyboard.append(["Done", "All"])

    markup_device = ReplyKeyboardMarkup(device_keyboard, resize_keyboard=True, one_time_keyboard=True)

    return markup_device

def start(update, context):
    context.bot.send_message(text="Hello! I'm here to control your home. Send /help to see what I can do.", chat_id=update.effective_chat.id)


def help(update, context):
    context.bot.send_message(text="I am a bot. You can use me to control your home.\n\nCommand available:\n/start to start the bot\n/devices to control a device\n/overview to take an overview on all your devices", chat_id=update.effective_chat.id) #da aggiornare con tutti i comandi

def overview(update, context):
    r = requests.get("http://localhost:8080/catalog/devices/all")
    if r.status_code != 200:
        print("HTTP Error %s" %r.status_code)
    data = json.loads(r.content)

    summary = []
    for device in data["devices"]:
        summary.append("%s: %s" %(device["deviceID"], device["values"]))

    context.bot.send_message(text="\n".join(summary), chat_id=update.effective_chat.id)

def devices(update, context):
    update.message.reply_text("Choose the device:", reply_markup= getMkpDevice())
    return CHOOSING_DEVICE

def manageDevice(update, context):
    context.user_data["chosen_device_id"] = update.message.text
    update.message.reply_text("You have chosen %s\nGive me a command!" %context.user_data["chosen_device_id"].casefold(), reply_markup= getMkpCommand(context.user_data["chosen_device_id"]))
    return DEVICE_OPERATION

def turnLight(update, context):
    value = update.message.text
    bool_value = True if value == "On" else False
    payload = {
        "deviceID": context.user_data["chosen_device_id"],
        "resources": ["illumination"],
        "values": [bool_value],
        "units": [None]
    }
    endpoint = "http://localhost:8080/catalog/update"
    r = requests.put(endpoint, data= json.dumps(payload))
    if r.status_code >= 400:
        print("HTTP Error.")
    update.message.reply_text("%s %s.\nChoose another device:" %(context.user_data["chosen_device_id"],value.casefold()), reply_markup= getMkpDevice())
    return CHOOSING_DEVICE

def readInfo(update, context):
    r = requests.get("http://localhost:8080/catalog/devices/%s" %context.user_data["chosen_device_id"])
    if r.status_code != 200:
        print("HTTP Error %s" %r.status_code)
    device = json.loads(r.content)

    # device = next(x for x in data["devices"] if x["deviceID"] == context.user_data["chosen_device_id"])
    resources = device["resources"]
    ind = resources.index(update.message.text)
    value = device["values"][ind]
    unit = device["units"][ind]

    update.message.reply_text("%s: %s %s.\nChoose another device:" %(update.message.text, value, unit), reply_markup= getMkpDevice())
    return CHOOSING_DEVICE


def done(update, context):
    context.user_data.clear()
    update.message.reply_text("Bye!")
    return ConversationHandler.END

def boot():
    print("Bot starting...")
    updater = Updater(token=TOKEN, use_context=True)
    dispatcher = updater.dispatcher

    conv_handler = ConversationHandler(
    entry_points=[CommandHandler("devices", devices)],

    states={
    CHOOSING_DEVICE: [MessageHandler(Filters.regex("^Done$"), done),    MessageHandler(Filters.text, manageDevice)],
    DEVICE_OPERATION: [MessageHandler(Filters.regex("^(On|Off)$"), turnLight), MessageHandler(Filters.text, readInfo)]
    },

    fallbacks=[MessageHandler(Filters.regex("^Done$"), done)]
    )

    dispatcher.add_handler(conv_handler)
    dispatcher.add_handler(CommandHandler("help", help))
    dispatcher.add_handler(CommandHandler("overview", overview), group=1)
    dispatcher.add_handler(CommandHandler("start", start), group=2)
    print("Bot ready.")

    updater.start_polling(clean= True)

if __name__ == "__main__":
    boot()
