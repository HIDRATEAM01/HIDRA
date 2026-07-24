# iqa_calculator.py
import math
import numpy as np

# Pesos dos parâmetros segundo CETESB
PESOS = {
    'OD': 0.17,
    'DBO': 0.10,
    'Coliformes': 0.15,
    'pH': 0.12,
    'NT': 0.10,
    'FT': 0.10,
    'Temperatura': 0.10,
    'Turbidez': 0.08,
    'Residuos': 0.08
}

class IQACalculator:
    """Calculadora do Índice de Qualidade da Água (IQA) segundo CETESB"""
    
    @staticmethod
    def curva_Coliformes(coliformes):
        """Curva de qualidade para Coliformes Termotolerantes"""
        A1 = 98.03
        B1 = -36.45
        C1 = 3.138
        D1 = 0.06776
        log_cf = math.log10(coliformes)
        return A1 + (B1 * log_cf) + (C1 * (log_cf**2)) + (D1*(log_cf**3))

    @staticmethod
    def curva_pH(ph):
        """Curva de qualidade para pH"""
        A2 = 0.05421
        B2 = 1.23
        C2 = -0.09873
        return A2 * ph ** ((B2*ph)+(C2*(ph**2)))+5.213

    @staticmethod
    def curva_DBO(dbo):
        """Curva de qualidade para DBO"""
        A3 = 102.6
        B3 = -0.1101
        return A3 * np.exp(B3 * dbo)

    @staticmethod
    def curva_NT(nt):
        """Curva de qualidade para Nitrogênio Total"""
        A4 = 98.96
        B4 = -0.2232
        C4 = -0.006457
        return A4 * (nt ** (B4 + C4 * nt))

    @staticmethod
    def curva_FT(ft):
        """Curva de qualidade para Fósforo Total"""
        A5 = 213.7
        B5 = -1.680
        C5 = 0.3325
        return A5 * math.exp(B5 * (ft ** C5))

    @staticmethod
    def curva_Temperatura(delta_temp):
        """Curva de qualidade para Temperatura (diferença da ambiente)"""
        A6 = 0.0003869
        B6 = 0.1815
        C6 = 0.01081
        return 1 / (A6 * ((delta_temp + B6) ** 2) + C6)

    @staticmethod
    def curva_Turbidez(turb):
        """Curva de qualidade para Turbidez"""
        A7 = 97.34
        B7 = -0.01139
        C7 = -0.04917
        return A7 * math.exp(B7 * turb + C7 * math.sqrt(turb))

    @staticmethod
    def curva_Residuos(rt):
        """Curva de qualidade para Resíduos Totais"""
        A8 = 80.26
        B8 = -0.00107
        C8 = 0.03009
        D8 = -0.1185
        return A8 * math.exp(B8 * rt + C8 * math.sqrt(rt)) + D8 * rt

    @staticmethod
    def curva_OD(od):
        """Curva de qualidade para Oxigênio Dissolvido"""
        A9 = 100.8
        B9 = -106
        C9 = -3745
        return A9 * math.exp(((od + B9) ** 2) / C9)

    @classmethod
    def calcular_IQA(cls, valores):
        """
        Calcula o IQA com base nos valores dos parâmetros
        
        Args:
            valores (dict): Dicionário com os valores dos parâmetros
                - Coliformes: NMP/100mL
                - pH: unidades de pH
                - DBO: mg/L
                - NT: mg/L (Nitrogênio Total)
                - FT: mg/L (Fósforo Total)
                - Temperatura: ΔT (diferença da temperatura ambiente)
                - Turbidez: NTU
                - Residuos: mg/L (Sólidos Totais)
                - OD: mg/L (Oxigênio Dissolvido)
        
        Returns:
            tuple: (iqa_valor, subindices_dict)
        """
        try:
            subindices = {
                'Coliformes': cls.curva_Coliformes(valores['Coliformes']),
                'pH': cls.curva_pH(valores['pH']),
                'DBO': cls.curva_DBO(valores['DBO']),
                'NT': cls.curva_NT(valores['NT']),
                'FT': cls.curva_FT(valores['FT']),
                'Temperatura': cls.curva_Temperatura(valores['Temperatura']),
                'Turbidez': cls.curva_Turbidez(valores['Turbidez']),
                'Residuos': cls.curva_Residuos(valores['Residuos']),
                'OD': cls.curva_OD(valores['OD'])
            }

            iqa = 1
            for param, qi in subindices.items():
                iqa *= qi ** PESOS[param]

            return round(iqa, 2), subindices
        
        except Exception as e:
            print(f"Erro no cálculo do IQA: {e}")
            return 0, {}

    @staticmethod
    def classificar_IQA(iqa):
        """
        Classifica o IQA segundo padrões CETESB
        
        Args:
            iqa (float): Valor do IQA
            
        Returns:
            tuple: (classificacao, css_class)
        """
        if iqa > 79:
            return "Ótima", "status-excellent"
        elif iqa > 51:
            return "Boa", "status-good"
        elif iqa > 36:
            return "Regular", "status-warning"
        elif iqa > 19:
            return "Ruim", "status-bad"
        else:
            return "Péssima", "status-critical"

    @classmethod
    def get_parametros_alertas(cls, valores):
        """
        Identifica parâmetros que estão fora dos limites recomendados
        
        Args:
            valores (dict): Valores dos parâmetros
            
        Returns:
            dict: Dicionário com alertas por parâmetro
        """
        alertas = {}
        
        # Definir limites para alertas
        limites = {
            'pH': {'min': 6.0, 'max': 9.0, 'ideal_min': 6.5, 'ideal_max': 8.5},
            'OD': {'min': 4.0, 'ideal_min': 6.0},
            'DBO': {'max': 5.0, 'critical': 10.0},
            'Coliformes': {'max': 100, 'critical': 1000},
            'NT': {'max': 2.18, 'critical': 3.7},
            'FT': {'max': 0.1, 'critical': 0.15},
            'Turbidez': {'max': 40, 'critical': 100},
            'Residuos': {'max': 300, 'critical': 500}
        }
        
        # Verificar cada parâmetro
        for param, valor in valores.items():
            if param in limites:
                limite = limites[param]
                
                if 'critical' in limite and valor > limite['critical']:
                    alertas[param] = 'critical'
                elif 'max' in limite and valor > limite['max']:
                    alertas[param] = 'warning'
                elif 'min' in limite and valor < limite['min']:
                    alertas[param] = 'critical'
                elif 'ideal_min' in limite and valor < limite['ideal_min']:
                    alertas[param] = 'warning'
                elif 'ideal_max' in limite and valor > limite['ideal_max']:
                    alertas[param] = 'warning'
                else:
                    alertas[param] = 'normal'
        
        return alertas


# Função de conveniência para manter compatibilidade com código existente
def calcular_IQA(valores):
    """Função wrapper para manter compatibilidade"""
    return IQACalculator.calcular_IQA(valores)

def classificar_IQA(iqa):
    """Função wrapper para manter compatibilidade"""
    return IQACalculator.classificar_IQA(iqa)


# Exemplo de uso atualizado
if __name__ == "__main__":
    valores_exemplo = {
        'Coliformes': 33.96,     # NMP/100mL
        'pH': 8.41,             # UNT
        'DBO': 6.06,            # mg/L
        'NT': 2.10,             # mg/L
        'FT': 0.22,            # mg/L
        'Temperatura': 2,    # ∆T (diferença da temperatura da água e a ambiente)
        'Turbidez': 22.62,       # UNT
        'Residuos': 255.75,      # mg/L
        'OD': 7.28             # mg/L
    }

    calculator = IQACalculator()
    iqa, subs = calculator.calcular_IQA(valores_exemplo)
    classificacao, css_class = calculator.classificar_IQA(iqa)
    alertas = calculator.get_parametros_alertas(valores_exemplo)

    print("=" * 60)
    print("RELATÓRIO DE QUALIDADE DA ÁGUA - IQA")
    print("=" * 60)
    print(f"IQA Final: {iqa} → Classificação: {classificacao}")
    print("=" * 60)
    
    print("\nParâmetro       Valor Exemplo      Subíndice    Status")
    print("-" * 60)
    for k in valores_exemplo:
        valor = valores_exemplo[k]
        subindice = subs[k]
        status = alertas.get(k, 'N/A')
        print(f"{k:<15} {valor:<18} {subindice:>8.2f}    {status}")
    
    print("\n" + "=" * 60)