import numpy as np
import pickle
import pandas as pd
import matplotlib.pyplot as plt
import math
from sklearn.ensemble import RandomForestClassifier
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import LabelEncoder
from sklearn.metrics import classification_report, accuracy_score
import seaborn as sns

filename = 'model.pkl'

# Load the model
with open(filename, 'rb') as file:
    loaded_model = pickle.load(file)

# Dados de entrada
X_new = [[
    15.5,    # Coloração
    15.0,   # Condutividade
    14,     # pH
    25.0,    # Temperatura da Água
    3.8      # Turbidez
]]

# Converter para DataFrame com os nomes corretos das features
X = pd.DataFrame(X_new, columns=[
    'Coloração',
    'Condutividade',
    'pH',
    'Temperatura da Água',
    'Turbidez'
])

# Use the loaded model
predictions = loaded_model.predict(X)

print(predictions)