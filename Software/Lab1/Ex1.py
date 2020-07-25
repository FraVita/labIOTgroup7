import cherrypy
import json

@cherrypy.expose
class WebService:
    def convert(self, originalUnit, targetUnit, value):
        if not(targetUnit != "f" or "c" or "k"):
            raise cherrypy.HTTPError(400, "Bad Request, (targetUnit c, f or k)")

        if targetUnit == originalUnit:
            return value

        if originalUnit == "c":
            if targetUnit == "f":
                return value*9/5 + 32

            elif targetUnit == "k":
                return value + 273.15

        elif originalUnit == "f":
            if targetUnit == "c":
                return (value - 32)*5/9

            elif targetUnit == "k":
                return (value - 32)*5/9 + 273.15

        elif originalUnit == "k":
            if targetUnit == "f":
                return (value - 273.15)*9/5 + 32

            elif targetUnit == "c":
                return value - 273.15

        else:
            raise cherrypy.HTTPError(400, "Bad Request, (originalUnit c, f or k)")

    def GET(self, *uri, **params):

        if len(params) == 3:
            try:
                value = params["value"].casefold()
                originalUnit = params["originalUnit"].casefold()
                targetUnit = params["targetUnit"].casefold()
            except KeyError:
                raise cherrypy.HTTPError(400, "Key-values")

            converted = self.convert(originalUnit, targetUnit, int(value))

            dict = {
            "orignalValue": value,
            "originalUnit": originalUnit.upper(),
            "targetValue": converted,
            "targetUnit": targetUnit.upper()
            }

            return json.dumps(dict, indent=4)

        else:
            raise cherrypy.HTTPError(400, "Bad Request, (params) ")




if __name__ == "__main__":
    conf = {
        "/": {
            'request.dispatch': cherrypy.dispatch.MethodDispatcher(),
        }
    }

    cherrypy.tree.mount(WebService(), "/converter", conf)
    cherrypy.engine.start()
    cherrypy.engine.block()
