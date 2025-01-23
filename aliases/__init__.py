# -*- coding: utf-8 -*-

import subprocess

from albert import *

md_iid = "2.3"
md_version = "1.0"
md_name = "Bash Aliases"
md_description = "Run bash command line aliases"
md_license = "MIT"
md_url = "https://github.com/albertlauncher/python/tree/main/aliases"
md_authors = "@perdiesman"
md_bin_dependencies = "/bin/bash"


class Plugin(PluginInstance, TriggerQueryHandler):
    def __init__(self):
        PluginInstance.__init__(self)
        TriggerQueryHandler.__init__(
            self, self.id, self.name, self.description,
            defaultTrigger='a ',
            synopsis='<alias name>'
        )

    def handleTriggerQuery(self, query):
        items = []

        try:
            cmd_result = subprocess.run(['/bin/bash', '-i', '-c', 'alias'], stdout=subprocess.PIPE)
            with open('/tmp/aliases.txt', 'w') as f:
                f.write(cmd_result.stdout.decode('utf-8'))
            aliases = cmd_result.stdout.decode('utf-8').split('\n')
            for alias in aliases:
                if alias != '' and query.string in alias.split(' ', 1)[1].split('=')[0]:
                    items.append(StandardItem(
                        id=alias.split(' ', 1)[1].split('=')[0],
                        text=alias.split(' ', 1)[1].split('=')[0],
                        subtext=alias.split(' ', 1)[1].split('=', 1)[1],
                        actions=[Action("run", "Run", lambda a=alias.split(' ', 1)[1].split('=')[0]: runTerminal(a))]
                    ))
        except Exception as e:
            warning(str(e))

        query.add(items)
