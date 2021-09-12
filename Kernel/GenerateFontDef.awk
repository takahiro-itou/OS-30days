
BEGIN {
    code=0;
}

/^char 0x/ {
    code=strtonum($2);
}

/^[.*]{8}$/ {
    text = $1;
    val  = 0;
    for (i = 0; i < 8; ++ i) {
        val *= 2;
        if (substr(text, i + 1, 1) == "*") {
            val += 1;
        }
    }
    data[code] = data[code] sprintf("0x%02x, ", val);
}

END {
    printf("static const char hankaku[4096] = {\n");
    for (c = 0; c < 255; ++ c ) {
        printf("%.94s, \t/* 0x%02x */\n", data[c], c);
    }
        printf("%.94s  \t/* 0x%02x */\n", data[c], c);
    printf("};\n");
}
