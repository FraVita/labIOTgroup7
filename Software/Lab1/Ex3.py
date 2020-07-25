import cherrypy
import json

@cherrypy.expose
class WebService:
    def convert(self, originalUnit, targetUnit, values):
        if not(targetUnit != "f" or "c" or "k"):
            raise cherrypy.HTTPError(400, "Bad Request, (targetUnit c, f or k)")

        converted = []
        if targetUnit == originalUnit:
            return values

        for value in values:
            if originalUnit == "c":
                if targetUnit == "f":
                    converted.append(value*9/5 + 32)

                elif targetUnit == "k":
                    converted.append(value + 273.15)

            elif originalUnit == "f":
                if targetUnit == "c":
                    converted.append((value - 32)*5/9)

                elif targetUnit == "k":
                    converted.append((value - 32)*5/9 + 273.15)

            elif originalUnit == "k":
                if targetUnit == "f":
                    converted.append((value - 273.15)*9/5 + 32)

                elif targetUnit == "c":
                    converted.append(value - 273.15)

            else:
                raise cherrypy.HTTPError(400, "Bad Request, (originalUnit c, f or k)")

        return converted

    def PUT(self, *uri, **params):
        self.content = json.loads(cherrypy.request.body.read())

        values = self.content["values"]
        originalUnit = self.content["originalUnit"].casefold()
        targetUnit = self.content["targetUnit"].casefold()

        converted = self.convert(originalUnit, targetUnit, values)

        dict = {
        "orignalValue": values,
        "originalUnit": originalUnit.upper(),
        "targetValue": converted,
        "targetUnit": targetUnit.upper()
        }

        return json.dumps(dict, indent=4)


if __name__ == "__main__":
    conf = {
        "/": {
            'request.dispatch': cherrypy.dispatch.MethodDispatcher(),
        }
    }

    cherrypy.tree.mount(WebService(), "/converter", conf)
    cherrypy.engine.start()
    cherrypy.engine.block()
