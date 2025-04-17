import streamlit as st
import openai

# Ganti dengan API key kamu
openai.api_key = "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"

st.title("Asisten Energi Surya ⚡☀️")
st.write("Tanyakan apa saja seputar energi surya — panel surya, efisiensi, teknologi, pemasangan, dan lainnya!")

user_input = st.text_input("Pertanyaan kamu:", "")

if user_input:
    with st.spinner("Menjawab pertanyaan kamu..."):
        try:
            response = openai.ChatCompletion.create(
                model="gpt-4o-mini",
                messages=[
                    {
                        "role": "system",
                        "content": (
                            "Kamu adalah asisten ahli di bidang energi surya. "
                            "Tugasmu adalah membantu pengguna memahami konsep, teknologi, pemasangan, dan penggunaan energi matahari. "
                            "Jawaban harus jelas, edukatif, dan jika perlu sertakan contoh atau analogi sederhana."
                        )
                    },
                    {"role": "user", "content": user_input}
                ]
            )
            st.success("Jawaban:")  
            st.write(response["choices"][0]["message"]["content"])
        except Exception as e:
            st.error(f"Terjadi kesalahan: {e}")
