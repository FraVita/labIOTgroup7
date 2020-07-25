import cherrypy
import json, os
from jinja2 import Environment, FileSystemLoader

@cherrypy.expose
class WebService:
    def GET(self, *uri, **params):
        loader = FileSystemLoader("freeboard")
        env = Environment(loader=loader)

        templ = env.get_template('index.html')
        output = templ.render()

        return output

    def POST(self, *uri, **params):
        cont = params['json_string']
        if uri[0] == "saveDashboard":
            with open("freeboard/dashboard/dashboard.json", "w") as f:
                f.write(cont)


@cherrypy.expose
class Dashboard:
    def GET(self, *uri, **params):
        with open("freeboard/dashboard/dashboard.json", "rb") as f:
            j = json.load(f)
            js = json.dumps(j)
            print(js)
            return js


if __name__ == "__main__":
    conf = {
        "/": {
            'request.dispatch': cherrypy.dispatch.MethodDispatcher(),
            'tools.staticdir.root': os.path.abspath(os.getcwd()),
            'tools.staticdir.on': True,
            'tools.staticdir.dir': './freeboard'
        },
    }

    confDash = {
        "/": {
            'request.dispatch': cherrypy.dispatch.MethodDispatcher()
        },
    }

    cherrypy.tree.mount(WebService(), "/", conf)
    cherrypy.tree.mount(Dashboard(), "/static/dashboard/dashboard.json", confDash)

    cherrypy.engine.start()
    cherrypy.engine.block()
