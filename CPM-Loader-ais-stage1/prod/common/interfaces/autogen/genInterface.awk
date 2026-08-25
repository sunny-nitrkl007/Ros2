{
    tokens["\\${DataType}"] = DataType
    tokens["\\${UseCsvable}"] = UseCsvable
    tokens["\\${UseTypeTraits}"] = UseTypeTraits
    tokens["\\${UseTextWritable}"] = UseTextWritable
    tokens["\\${UseRestorable}"] = UseRestorable
    tokens["\\${PathToDataType}"] = PathToDataType
    tokens["\\${InOut}"] = InOut
    tokens["\\${IntfName}"] = IntfName ".Channel." InOut
    tokens["\\${Year}"] = Year

    for ( x in tokens ) {
        gsub(x, tokens[x]);
    }

    gsub(/\${Filename}/, "'interfaces/autogen/generate'");

    gsub(/\${Filename}/, "'interfaces/autogen/generate'");

    print
}

