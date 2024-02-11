// src/widgets/widget/dynamic/fields.ts
class DynamicField {
  value = "";
  field;
  constructor(field) {
    this.field = field;
  }
  get_value() {
    return this.value;
  }
  set_value(value) {
    this.value = value;
    this.field.innerText = value;
  }
}

// src/widgets/widget/dynamic/field_container.ts
class AbstractFieldContainer {
  fields;
  widget_node;
  constructor(widget_node, data_attribute_name) {
    this.widget_node = widget_node;
    this.fields = this.locate_fields(data_attribute_name);
  }
  locate_fields(data_attribute_name) {
    var html_fields = this.widget_node.querySelectorAll(`[data-field="${data_attribute_name}"]`);
    console.log(html_fields);
    var logical_fields = new Map;
    for (var index = 0;index < html_fields.length; index++) {
      const field = html_fields[index];
      var field_id = field.id;
      var field_object = this.create_field(field);
      logical_fields.set(field_id, field_object);
    }
    return logical_fields;
  }
  set_field_values(field_data) {
    for (const [key, field] of this.fields.entries()) {
      if (field_data.hasOwnProperty(key)) {
        var value_to_set = field_data[key];
        field.set_value(value_to_set);
      }
    }
  }
  get_field_values() {
    var values_map = {};
    for (const [key, field] of this.fields.entries()) {
      values_map[key] = field.get_value();
    }
    return values_map;
  }
}

class DynamicFields extends AbstractFieldContainer {
  constructor(widget_node) {
    super(widget_node, "dynamic");
  }
  create_field(field_node) {
    return new DynamicField(field_node);
  }
}

// src/widgets/widget/dynamic/silly.ts
console.log("hehe");
var testing_fields = new DynamicFields(document.getElementById("container") ?? function() {
  throw new Error("THE SELECTED ID WAS NOT FOUND");
}());
console.log(testing_fields.get_field_values());
