(
    (comment)? @field-comment
    .
    (field_declaration
        (attribute_declaration
            (attribute
            prefix: (identifier) @field-attr-prefix
            name: (identifier) @field-attr-name (#eq? @field-attr-name "Export")))
        declarator: (field_identifier) @field-name)
)