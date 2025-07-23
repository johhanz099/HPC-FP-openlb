EXAMPLE = cylinder2d
OLB_ROOT := ..
include $(OLB_ROOT)/default.mk

# Direactorios
DAT = data
FIG = fig

run_simul: $(EXAMPLE)
	@echo
	@read -p "  >> Ingrese el número de procesos para ejecutar el programa: " N; \
	echo; \
	echo "  Ejecutando $(EXAMPLE) con $$N procesos"; \
	mpirun --use-hwthread-cpus -np $$N ./$(EXAMPLE); \
	echo "Programa ejecutado con $$N procesos."

metrics: $(EXAMPLE)
	bash run.sh
	@echo "Generando gráficas de métricas en carpeta $(FIG)"
	gnuplot metrics.gp

clean_simul:
	@echo "Cleaning /$(DAT)"
	rm -f $(DAT)/*.log $(DAT)/*.csv
	@echo "Cleaning /$(TMP)"
	rm -f $(FIG)/*.png