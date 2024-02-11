import { DynamicFields } from "./src/widgets/widget/dynamic/field_container";
console.log("hehe");
var testing_fields: DynamicFields = new DynamicFields(document.getElementById("container") ?? function(){throw new Error("THE SELECTED ID WAS NOT FOUND")}());

console.log(testing_fields.get_field_values());