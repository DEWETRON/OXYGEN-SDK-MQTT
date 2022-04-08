from json_schema_for_humans.generate import generate_from_filename
from json_schema_for_humans.generation_configuration import GenerationConfiguration
from jsonschema import validate
import jsonref
import json
import pathlib
import jinja2

base_dir = pathlib.Path(__file__).parent.absolute()

def get_env(path):
    global env

    env = jinja2.Environment(
        keep_trailing_newline=True,
        block_start_string="/*%",
        block_end_string="%*/",
        variable_start_string="/*{",
        variable_end_string="}*/",
        line_statement_prefix="//#",
        line_comment_prefix="//##",
        autoescape=False,
        loader=jinja2.FileSystemLoader(path),
    )

    return env

# Build documentation
config = GenerationConfiguration(expand_buttons=True)
generate_from_filename(base_dir / "schema.json", "./docs/schema.html", config=config)

# Validate the example-schema
schema = ""
example = ""

with open(base_dir / "schema.json", "r") as f:
    schema = json.load(f)

with open(base_dir / "example.json", "r") as f:
    # Load example and dereference $ref to ensure jsonschema also validates references
    example = jsonref.load(f)

validate(instance=example, schema=schema)

# Use JINJA2 Template engine to update schema.h for C/C++ Code
env = get_env(base_dir / "templates")
template = env.get_template("Schema.h.jinja")
rendered = template.render({"schema": json.dumps(schema, indent=4)})

with open("./mqtt-plugin/include/configuration/details/Schema.h", "w") as f:
    f.write(rendered)
