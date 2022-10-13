#
# 12-Oct-22 CBL   Response is pretty Dull
#
# I don't think I'm picking up the template. 
#
#  https://flask-menu.readthedocs.io/en/latest/
#
# ===========================================================

from flask import Flask
from flask import render_template_string
from flask_menu import Menu, register_menu

app = Flask(__name__)
Menu(app=app)

def tmpl_show_menu():
    return render_template_string(
        """
        {%- for item in current_menu.children %}
            {% if item.active %}*{% endif %}{{ item.text }}
        {% endfor -%}
        """)

@app.route('/')
@register_menu(app, '.', 'Home')
def index():
    return tmpl_show_menu()

@app.route('/first')
@register_menu(app, '.first', 'First', order=0)
def first():
    return tmpl_show_menu()

@app.route('/second')
@register_menu(app, '.second', 'Second', order=1)
def second():
    return tmpl_show_menu()

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
    
