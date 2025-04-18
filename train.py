import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn.ensemble import RandomForestRegressor
from sklearn.metrics import mean_squared_error, r2_score
import joblib

df = pd.read_csv("solar_dataset.csv")

X = df[['hour', 'minute', 'dayofweek', 'ldr', 'servo_v', 'servo_h']]
y = df['mwh']

X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)

model = RandomForestRegressor(n_estimators=100, random_state=42)
model.fit(X_train, y_train)

y_pred = model.predict(X_test)

mse = mean_squared_error(y_test, y_pred)
r2 = r2_score(y_test, y_pred)

print("Mean Squared Error:", mse)
print("R² Score:", r2)

# 6. Simpan Model
joblib.dump(model, "mwh_predictor.pkl")
print("Model berhasil disimpan ke 'mwh_predictor.pkl'")
