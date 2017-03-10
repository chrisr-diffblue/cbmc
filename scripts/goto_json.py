
import os
import os.path
import json
import copy
import sys

class GotoJsonException(BaseException):
  pass

class NoSuchIrepKeyException(GotoJsonException):
  pass

class irep(object):

  def __init__(self, schema_name, schema, initial_id = ""):
    self.sub = []
    self._id = initial_id
    self.namedSub = dict()
    self.comment = dict()
    self.schema_name = schema_name
    self.schema = schema

  def find_friendly_name(self, key):
    if "sub" not in self.schema:
      return None
    for (i, sub) in enumerate(self.schema["sub"]):
      if "friendly_name" in sub:
        name = sub["friendly_name"]
        if isinstance(name, list):
          names = name
        else:
          names = [name]
        for name in names:
          if name == key:
            return i
          elif "number" in sub and sub["number"] == "*" and name + "s" == key:
            return slice(i)
    return None

  def get_key_with_schema(self, key, schema):

    def irep_to_schema_type(spec, value):
      assert isinstance(value, irep)
      if "type" in spec:
        if spec["type"] == "string":
          return value._id
        elif spec["type"] == "integer":
          return int(value._id)
        elif spec["type"] == "boolean":
          if value._id == "0":
            return False
          elif value._id == "1":
            return True
          else:
            raise GotoJsonException("Unexpected value of bool-typed field '%s'" % value.id)
        else:
          raise GotoJsonException("Unexpected spec type '%s'" % spec["type"])
      else:
        return value

    if schema is None:
      raise GotoJsonException("This irep doesn't have a schema. Access sub, namedSub and comment directly.")
    if "namedSub" in schema and key in schema["namedSub"]:
      return irep_to_schema_type(schema["namedSub"][key], self.namedSub[key])
    if "comment" in schema and key in schema["comment"]:
      return irep_to_schema_type(schema["comment"][key], self.comment[key])
    subidx = self.find_friendly_name(key)
    # TODO: implement typing against "number": "*" subexpressions if
    # that ever gets used.
    if subidx is not None:
      return self.sub[subidx]
    if "parent" in schema:
      try:
        return self.get_key_with_schema(key, schema["parent"])
      except NoSuchIrepKeyException:
        pass
    raise NoSuchIrepKeyException("'%s' not defined in the irep schema '%s'" % (key, schema_name))

  def __getitem__(self, key):
    return self.get_key_with_schema(key, self.schema)

  def set_key_with_schema(self, key, value, schema):

    def schema_type_to_irep(value, spec):
      if "type" in spec:
        if spec["type"] == "string":
          assert isinstance(value, str)
          return irep(None, None, value)
        elif spec["type"] == "integer":
          assert isinstance(value, int)
          return irep(None, None, str(value))
        elif spec["type"] == "bool":
          assert isinstance(value, bool)
          return irep(None, None, "1" if value else "0")
        else:
          raise GotoJsonException("Unexpected spec type '%s'" % spec["type"])
      else:
        if not isinstance(value, irep):
          raise GotoJsonException("Expected irep, got %s" % str(value))
        return value

    if schema is None:
      raise NoSuchIrepKeyException("This irep doesn't have a schema. Access sub, namedSub and comment directly.")
    if "namedSub" in schema and key in schema["namedSub"]:
      self.namedSub[key] = schema_type_to_irep(value, schema["namedSub"][key])
      return
    if "comment" in schema and key in schema["comment"]:
      self.comment[key] = schema_type_to_irep(value, schema["comment"][key])
      return
    subidx = self.find_friendly_name(key)
    if subidx is not None:
      if isinstance(subidx, int):
        while len(self.sub) <= subidx:
          self.sub.append(None)
      self.sub[subidx] = value
      return
    if "parent" in schema:
      try:
        self.set_key_with_schema(key, value, schema["parent"])
        return
      except NoSuchIrepKeyException:
        pass
    raise NoSuchIrepKeyException("'%s' is not defined in the irep schema '%s'" % (key, self.schema_name))

  def __setitem__(self, key, value):
    self.set_key_with_schema(key, value, self.schema)

  def check_schema(self):
    def warning(s):
      print >>sys.stderr, "irep(%s): %s" % (self.schema_name, s)
    if self.schema is None:
      # Nothing to check
      return
    if self._id == "":
      warning("has a schema, but no id")
    expected_subs = 0
    if "sub" in self.schema:
      for subspec in self.schema["sub"]:
        if "number" not in subspec:
          expected_subs += 1
    if len(self.sub) < expected_subs:
      warning("expected at least %d subexpressions (have %d)" % (expected_subs, len(self.sub)))
    if "namedSub" in self.schema:
      for k in self.schema["namedSub"]:
        if k not in self.namedSub:
          warning("expected named subexpression '%s'" % k)
    if "comment" in self.schema:
      for k in self.schema["comment"]:
        if k not in self.comment:
          warning("expected named subexpression '%s'" % k)

  def to_json_serialisable_dict(self):
    self.check_schema()
    return {"id": self._id,
            "sub": [subexpr.to_json_serialisable_dict() for subexpr in self.sub],
            "namedSub": {k: v.to_json_serialisable_dict() for (k, v) in self.namedSub.iteritems()},
            "comment": {"#"+k: v.to_json_serialisable_dict() for (k, v) in self.comment.iteritems()}}

class goto_json(object):

  def __init__(self, spec_dir):
    schemata = dict()
    for f in os.listdir(spec_dir):
      if f.endswith(".json"):
        schema_name = f[:-len(".json")]
        f = os.path.join(spec_dir, f)
        try:
          with open(f, "r") as fp:
            schema = json.load(fp)
            schemata[schema_name] = schema
            if isinstance(schema, dict) and "trivial_subclass_ids" in schema:
              for subclass_id in schema["trivial_subclass_ids"]:
                subschema = copy.copy(schema)
                subschema["id"] = subclass_id
                schemata[subclass_id] = subschema
        except Exception as e:
          raise GotoJsonException("Trying to parse %s: %s" % (f, str(e)))
    self.schemata = schemata
    for s in schemata.iteritems():
      self.check_schema(*s)
    # Give schemata direct pointers to their parent:
    for s in schemata.itervalues():
      if "parent" in s:
        s["parent"] = schemata[s["parent"]]

  def check_schema(self, sname, s):

    def fatal(err):
      raise GotoJsonException("Schema %s: %s" % (sname, err))

    def check_subexpr(subexpr):
      if "type" in subexpr:
        if subexpr["type"] not in ("integer", "string", "bool"):
          fatal("contains illegal type '%s'" % subexpr["type"])
      elif "schema" in subexpr:
        if subexpr["schema"] not in self.schemata:
          fatal("subexpression refers to non-existent schema '%s'" % subexpr["schema"])
      elif "constant" in subexpr:
        pass # Nothing to check as yet
      else:
        # Inline sub-irep.
        self.check_schema(sname, subexpr)

    if not isinstance(s, dict):
      fatal("top-level should be an object")
    if "parent" in s and s["parent"] not in self.schemata:
      fatal("refers to non-existent parent %s" % s["parent"])

    keys = set(s.keys())
    strange_keys = keys - set(["id",
                               "parent",
                               "sub",
                               "namedSub",
                               "comment",
                               "trivial_subclass_ids"])
    if len(strange_keys) != 0:
      fatal("irep contains unexpected keys %s" % strange_keys)

    if "sub" in s:
      if not isinstance(s["sub"], list):
        fatal("'sub' should be a list")
      for subexpr in s["sub"]:
        check_subexpr(subexpr)
    if "namedSub" in s:
      if not isinstance(s["namedSub"], dict):
        fatal("'namedSub' should be an object")
      for subexpr in s["namedSub"].itervalues():
        check_subexpr(subexpr)
    if "comment" in s:
      if not isinstance(s["comment"], dict):
        fatal("'comment' should be an object")
      for subexpr in s["comment"].itervalues():
        check_subexpr(subexpr)

  def make_constant_irep(self, constant):
    ret = irep(None, None)
    ret._id = constant
    return ret

  def find_schema_id(self, schema):
    if "id" in schema:
      return schema["id"]
    if "parent" in schema:
      return self.find_schema_id(schema["parent"])
    return ""

  def count_required_positional_operands(self, schema):
    if "sub" not in schema:
      return 0
    n = 0
    for s in schema["sub"]:
      if "number" not in s:
        n += 1
    if "parent" not in schema:
      return n
    else:
      return max(n, self.count_required_positional_operands(schema["parent"]))

  def make_irep(self, schema_name, **kwargs):

    schema = self.schemata[schema_name]
    ret = irep(schema_name, schema)
    # Apply any constants required by the schema:
    ret._id = self.find_schema_id(schema)
    if "namedSub" in schema:
      for (key, value) in schema["namedSub"].iteritems():
        if "constant" in value:
          ret.namedSub[key] = self.make_constant_irep(value["constant"])
    if "comment" in schema:
      for (key, value) in schema["comment"].iteritems():
        if "constant" in value:
          ret.comment[key] = self.make_constant_irep(value["constant"])
    # Apply any constructor args:
    for (key, value) in kwargs.iteritems():
      ret[key] = value

    return ret

class symbol_table(object):

  def __init__(self):
    self.symbols = dict()

  def add(self, sym):
    if sym["name"] in self.symbols:
      raise GotoJsonException("Duplicate symbol name '%s'" % sym["name"])
    self.symbols[sym["name"]] = sym

  def to_json(self):
    return json.dumps([s.to_json_serialisable_dict() for s in self.symbols.itervalues()])

class symbol(object):

  flags = [
    "is_type",
    "is_macro",
    "is_exported",
    "is_input",
    "is_output",
    "is_state_var",
    "is_property",
    "is_static_lifetime",
    "is_thread_local",
    "is_lvalue",
    "is_file_local",
    "is_extern",
    "is_volatile",
    "is_parameter",
    "is_auxiliary",
    "is_weak"
  ]

  strings = [
    "name",
    "module",
    "base_name",
    "mode",
    "pretty_name"
  ]

  ireps = [
    "type",
    "value",
    "location"
  ]

  def __init__(self, **kwargs):
    self.values = { k: False  for k in symbol.flags }
    self.values.update({ k: "" for k in symbol.strings })
    self.values.update({ k: irep(None, None, "nil") for k in symbol.ireps })
    # Use this instead of 'update' to typecheck:
    for (k, v) in kwargs.iteritems():
      self[k] = v;

  def __getitem__(self, key):
    return self.values[key]

  def __setitem__(self, key, value):
    if key in symbol.flags:
      assert(isinstance(value, bool))
    elif key in symbol.strings:
      assert(isinstance(value, str))
    elif key in symbol.ireps:
      assert(isinstance(value, irep))
    else:
      raise GotoJsonException("Unexpected symbol key '%s'" % key)
    self.values[key] = value

  def to_json_serialisable_dict(self):
    ret = copy.copy(self.values)
    ret.update({k: self.values[k].to_json_serialisable_dict() for k in symbol.ireps})
    return ret

  def symbol_expr(self, gj):
    return gj.make_irep("symbol_expr", identifier = self["name"], type = self["type"])
