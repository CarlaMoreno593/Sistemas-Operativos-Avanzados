struct comando {
	int tam;
	string elementos<>;

};

program COMANDO_PRG {
	version COMANDO_1 {
		string ejecucionComando(struct comando) = 1;
	} = 1;
} = 0x20000001;

