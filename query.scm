(class_specifier
  (attribute_declaration
    (attribute
      prefix: (identifier) @prefix
      name: (identifier) @name))
  name: (type_identifier) @class_name)

(
 (comment)? @the-comment
 (field_declaration
   (attribute_declaration
     (attribute
       prefix: (identifier) @field_attr_prefix
       name: (identifier) @field_attr_name))
   declarator: (field_identifier) @name)
 )
