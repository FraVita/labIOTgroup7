import cherrypy
import json


@cherrypy.expose
class WebService:

    content = []

    def GET(self, *uri, **params):
        return json.dumps(self.content, indent=4)

    def POST(self, *uri, **params):
        data = json.loads(cherrypy.request.body.read())
        self.content.append(data)
        print(data)
        return json.dumps(self.content, indent=4)


if __name__ == "__main__":
    conf = {
        "/": {
            'request.dispatch': cherrypy.dispatch.MethodDispatcher(),
        }
    }

    cherrypy.tree.mount(WebService(), "/log", conf)
    cherrypy.config.update({'server.socket_host': '0.0.0.0'})
    cherrypy.engine.start()
    cherrypy.engine.block()
